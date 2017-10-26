#include "p2p_connection.h"

#include <string>
#include <cstdint>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/asio.hpp>
#include "p2p_common.h"
#include "p2p_requests.h"

#include <iostream>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost_error = boost::system::error_code;

namespace p2p
{

const auto ANSWER_TIMEOUT = boost::posix_time::seconds(5);

connection::connection() : server_socket{service}, answer_timer{service}
{
}

connection::~connection()
{
    stop();
}

connection::ptr connection::create()
{
    connection *c = new connection{};
    return ptr{c};
}

void connection::connect(std::string address, uint16_t port)
{
    if (trying_to_connect || server_socket.is_open())
    {
        return;
    }

    server_endpoint = tcp::endpoint{ip::address::from_string(address), port};
    trying_to_connect = true;
    start();
}

void connection::wait_connection()
{
    unique_lock<mutex> lck(connection_mutex);
    connection_cond_var.wait(lck, [this]{ return !trying_to_connect; });
}

bool connection::answer_is_ready()
{
    if (answer_exception)
    {
        throw communication_exception{};
    }
    if (!is_connected())
    {
        throw disconnected_exception{};
    }
    return has_answer;
}

void connection::wait_answer()
{
    unique_lock<mutex> lck(answer_mutex);
    answer_cond_var.wait(lck, [this]{ return has_answer; });
    if (answer_exception)
    {
        throw communication_exception{};
    }
    if (!is_connected())
    {
        throw disconnected_exception{};
    }
}

void connection::send_request(std::unique_ptr<request> &&r)
{
    current_request = move(r);
    start_write();
}

void connection::start()
{
    service_thread = thread{[this]{service_thread_handler();}};
}

void connection::stop()
{
    work.reset();
    if (service_thread.joinable())
    {
        service_thread.join();
    }
}

void connection::service_thread_handler()
{
    server_socket.async_connect(server_endpoint,
        [this](boost_error ec)
        {
            if (!ec)
            {
                work = make_unique<io_service::work>(service);

                unique_lock<mutex> lck(connection_mutex);
                trying_to_connect = false;
                connection_cond_var.notify_all();
            }
        }
    );

    service.run();
}

void connection::close_connection(boost_error error)
{
    if (error.value() != 0)
    {
        cout << "error = " << error.value() << endl;
    }
    server_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    server_socket.close();
    unique_lock<mutex> lck(answer_mutex);
    has_answer = true;
    answer_cond_var.notify_all();
    stop();
    service.reset();
}

void connection::start_write()
{
    has_answer = false;
    answer_exception = false;
    current_request->fill_request(buf, buf_size);
    async_write(server_socket, buffer(buf, buf_size),
                [self = shared_from_this()](boost_error ec, size_t)
                { if (ec) self->close_connection(ec);
                  else self->start_read(); });
}

void connection::start_read()
{
    async_read(server_socket, buffer(buf),
               [self = shared_from_this()](boost_error error, size_t bytes)
               { return self->read_complete(error, bytes); },
               [self = shared_from_this()](boost_error error, size_t bytes)
               { self->read(error, bytes); });

    answer_timer.expires_from_now(ANSWER_TIMEOUT);
    answer_timer.async_wait([self = shared_from_this()](boost_error ec)
                            { if (ec != error::operation_aborted)
                              self->close_connection(ec); });
}

size_t connection::read_complete(boost::system::error_code error, size_t bytes)
{
    if (error)
    {
        close_connection(error);
        return 0;
    }

    return p2p::read_complete(buf, bytes);
}

void connection::read(boost::system::error_code error, size_t bytes)
{
    answer_timer.cancel();
    if (error)
    {
        close_connection(error);
        return;
    }

    if (p2p::is_valid_message(buf, bytes))
    {
        if (current_request->process_answer(buf, bytes))
        {
            unique_lock<mutex> lck(answer_mutex);
            has_answer = true;
            answer_cond_var.notify_all();
        }
        else
        {
            answer_exception = true;
            close_connection();
        }
    }
    else
    {
        answer_exception = true;
        close_connection();
    }
}

}
