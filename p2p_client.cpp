#include "p2p_client.h"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

#include "p2p_common.h"
#include "p2p_events.h"

using namespace std;

namespace p2p
{

constexpr int MAJOR = 0;
constexpr int MINOR = 0;
constexpr int PATCH = 0;

client::client() : con{connection::create()}
{
}

client::ptr client::create()
{
    auto cl = new client{};
    return ptr{cl};
}

string client::to_string(client::connection_result v)
{
    static map<client::connection_result, string> dict =
    {
        {client::connection_result::OK, "OK"},
        {client::connection_result::SHOULD_BE_UPDATE, "SHOULD_BE_UPDATE"},
        {client::connection_result::MUST_BE_UPDATE, "MUST_BE_UPDATE"},
        {client::connection_result::SERVER_OLD_VERSION, "SERVER_OLD_VERSION"},
        {client::connection_result::SERVER_INCOMPATIBLE_VERSION,
         "SERVER_INCOMPATIBLE_VERSION"},
        {client::connection_result::NOT_CONNECTED, "NOT_CONNECTED"},
        {client::connection_result::FAILED, "FAILED"},
    };

    try
    {
        return dict.at(v);
    }
    catch (const std::out_of_range&)
    {
        return "UNDEFINED";
    }
}

bool client::connect_to_server(string address, uint16_t port,
                               connection_result &result)
{
    if (con->is_connected())
    {
        result = connection_result::OK;
        return true;
    }

    con->connect(address, port);
    con->wait_connection();
    if (!con->is_connected())
    {
        result = connection_result::NOT_CONNECTED;
        return false;
    }

    result = connection_result::OK;
    return true;
}

}//p2p
