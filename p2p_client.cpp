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

client::client()
{
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

}//p2p
