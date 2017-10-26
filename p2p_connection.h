#ifndef P2P_CONNECTION_H
#define P2P_CONNECTION_H

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

namespace p2p
{

class connection : public std::enable_shared_from_this<connection>
{
    connection();

public:
    ~connection();
    using ptr = std::shared_ptr<connection>;
    static ptr create();

    void connect(std::string address, uint16_t port);
    void wait_connection();
    bool is_connected() const { return server_socket.is_open(); }

    struct disconnected_exception{};
    struct communication_exception{};
    bool answer_is_ready();
    void wait_answer();

    void send_request(std::unique_ptr<request> &&r);

    void close_connection(boost::system::error_code error =
            boost::system::error_code{boost::system::errc::success,
                                      boost::system::system_category()});

private:
    boost::asio::io_service service;
    std::unique_ptr<boost::asio::io_service::work> work;
    boost::asio::ip::tcp::socket server_socket;
    boost::asio::ip::tcp::endpoint server_endpoint;

    std::mutex connection_mutex;
    std::condition_variable connection_cond_var;

    void start();
    void stop();
    void service_thread_handler();
    std::thread service_thread;

    bool trying_to_connect = false;

    std::unique_ptr<request> current_request;
    buffer_type buf;
    size_t buf_size;
    std::mutex answer_mutex;
    std::condition_variable answer_cond_var;
    bool has_answer = false;
    bool answer_exception = false;
    boost::asio::deadline_timer answer_timer;
    void start_write();
    void start_read();
    size_t read_complete(boost::system::error_code error, size_t bytes);
    void read(boost::system::error_code error, size_t bytes);
};

}

#endif // CONNECTION_H
