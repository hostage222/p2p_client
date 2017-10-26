#include "p2p_client.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
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
    client_version = p2p::to_string(version_type{MAJOR, MINOR, PATCH});
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
        {connection_result::OK, "OK"},
        {connection_result::SHOULD_BE_UPDATE, "SHOULD_BE_UPDATE"},
        {connection_result::MUST_BE_UPDATE, "MUST_BE_UPDATE"},
        {connection_result::SERVER_OLD_VERSION, "SERVER_OLD_VERSION"},
        {connection_result::SERVER_INCOMPATIBLE_VERSION,
         "SERVER_INCOMPATIBLE_VERSION"},
        {connection_result::NOT_CONNECTED, "NOT_CONNECTED"},
        {connection_result::FAILED, "FAILED"},
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

    auto version = make_shared<version_type>();
    auto r = make_unique<get_version_request>(version);
    con->send_request(move(r));
    try
    {
        con->wait_answer();
    }
    catch (connection::communication_exception&)
    {
        result = connection_result::FAILED;
        return false;
    }
    catch (connection::disconnected_exception&)
    {
        result = connection_result::NOT_CONNECTED;
        return false;
    }

    if (MAJOR != version->major)
    {
        result = MAJOR < version->major ? connection_result::MUST_BE_UPDATE :
            connection_result::SERVER_INCOMPATIBLE_VERSION;
        con->close_connection();
        return false;
    }

    result = connection_result::OK;
    if (MINOR < version->minor)
    {
        result = connection_result::SHOULD_BE_UPDATE;
    }
    else if (MINOR > version->minor)
    {
        result = connection_result::SERVER_OLD_VERSION;
    }

    server_version = p2p::to_string(*version);

    return true;
}

client::version client::get_version()
{
    return client_version;
}

client::version client::get_server_version()
{
    return con->is_connected() ? server_version : "";
}

string client::to_string(client::register_result v)
{
    static map<client::register_result, string> dict =
    {
        {register_result::OK, "OK"},
        {register_result::NEED_CODE, "NEED_CODE"},
        {register_result::INVALID_CODE, "INVALID_CODE"},
        {register_result::ALREADY_EXISTS, "ALREADY_EXISTS"},
        {register_result::DISCONNECTED, "DISCONNECTED"},
        {register_result::FAILED, "FAILED"},
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

client::register_result client::register_on_server(string phone,
                                                   string password,
                                                   string code)
{
    static unordered_map<string, client::register_result> answer_dict =
    {
        {"OK", register_result::OK},
        {"NEED_CODE", register_result::NEED_CODE},
        {"INVALID_CODE", register_result::INVALID_CODE},
        {"ALREADY_EXISTS", register_result::ALREADY_EXISTS}
    };

    auto result_ptr = make_shared<string>();
    auto r = make_unique<register_request>(phone, password, code, result_ptr);
    return account_operation(answer_dict, std::move(r), result_ptr);
}

string client::to_string(client::unregister_result v)
{
    static map<client::unregister_result, string> dict =
    {
        {unregister_result::OK, "OK"},
        {unregister_result::INVALID_ACTION, "INVALID_ACTION"},
        {unregister_result::INVALID_PHONE, "INVALID_PHONE"},
        {unregister_result::INVALID_PASSWORD, "INVALID_PASSWORD"},
        {unregister_result::DISCONNECTED, "DISCONNECTED"},
        {unregister_result::FAILED, "FAILED"},
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

client::unregister_result client::unregister_on_server(string phone,
                                                       string password)
{
    static unordered_map<string, client::unregister_result> answer_dict =
    {
        {"OK", unregister_result::OK},
        {"INVALID_ACTION", unregister_result::INVALID_ACTION},
        {"INVALID_PHONE", unregister_result::INVALID_PHONE},
        {"INVALID_PASSWORD", unregister_result::INVALID_PASSWORD}
    };

    auto result_ptr = make_shared<string>();
    auto r = make_unique<unregister_request>(phone, password, result_ptr);
    return account_operation(answer_dict, std::move(r), result_ptr);
}

string client::to_string(client::autorize_result v)
{
    static map<client::autorize_result, string> dict =
    {
        {autorize_result::OK, "OK"},
        {autorize_result::INVALID_ACTION, "INVALID_ACTION"},
        {autorize_result::INVALID_PHONE, "INVALID_PHONE"},
        {autorize_result::INVALID_PASSWORD, "INVALID_PASSWORD"},
        {autorize_result::DISCONNECTED, "DISCONNECTED"},
        {autorize_result::FAILED, "FAILED"},
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

client::autorize_result client::autorize_on_server(string phone,
                                                   string password)
{
    static unordered_map<string, client::autorize_result> answer_dict =
    {
        {"OK", autorize_result::OK},
        {"INVALID_ACTION", autorize_result::INVALID_ACTION},
        {"INVALID_PHONE", autorize_result::INVALID_PHONE},
        {"INVALID_PASSWORD", autorize_result::INVALID_PASSWORD}
    };

    auto result_ptr = make_shared<string>();
    auto r = make_unique<autorize_request>(phone, password, result_ptr);
    return account_operation(answer_dict, std::move(r), result_ptr);
}

}//p2p
