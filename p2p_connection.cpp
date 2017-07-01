#include "p2p_connection.h"

#include <string>
#include <cstdint>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost_error = boost::system::error_code;

namespace p2p
{

connection::connection() : server_socket{service}
{
}

connection::~connection()
{
    work.reset();
    if (service_thread.joinable())
    {
        service_thread.join();
    }
}

connection::ptr connection::create()
{
    connection *c = new connection{};
    return ptr{c};
}

void connection::connect(std::string address, uint16_t port)
{
    if (trying_to_connect || is_connected_value)
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

void connection::start()
{
    service_thread = thread{[this]{service_thread_handler();}};
}

void connection::service_thread_handler()
{
    server_socket.async_connect(server_endpoint,
        [this](boost_error ec)
        {
            is_connected_value = !ec;

            work = make_unique<io_service::work>(service);

            unique_lock<mutex> lck(connection_mutex);
            trying_to_connect = false;
            connection_cond_var.notify_all();
        }
    );

    service.run();
}

}
