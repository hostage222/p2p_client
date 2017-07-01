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

namespace p2p
{

class connection : public std::enable_shared_from_this<connection>
{
    connection();

public:
    ~connection();
    using ptr = std::unique_ptr<connection>;
    static ptr create();

    void connect(std::string address, uint16_t port);
    void wait_connection();
    bool is_connected() const { return is_connected_value; }

private:
    boost::asio::io_service service;
    std::unique_ptr<boost::asio::io_service::work> work;
    boost::asio::ip::tcp::socket server_socket;
    boost::asio::ip::tcp::endpoint server_endpoint;

    std::mutex connection_mutex;
    std::condition_variable connection_cond_var;

    void start();
    void service_thread_handler();
    std::thread service_thread;

    std::atomic<bool> trying_to_connect{false};
    std::atomic<bool> is_connected_value{false};
    bool ready() const { return is_connected_value; }
};

}

#endif // CONNECTION_H
