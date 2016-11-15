#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "p2p_common.h"
#include "p2p_events.h"

namespace p2p
{

class client : public std::enable_shared_from_this<client>
{
public:
    using ptr = std::shared_ptr<client>;
    static ptr create();

    bool connect_to_server(std::string address);
    void close_all_connections();

    using phones_list = std::vector<std::string>;
    //Key = phone; Value = id
    using contacts_dictionary = std::map<std::string, std::string>;
    contacts_dictionary get_contacts(const phones_list &phones);

    event::ptr get_event();

    void connect_to_client(std::string friend_id);
    void close_client_connection(std::string friend_id);
    message_id_type send_message(std::string friend_id,
                                 const std::string &message);

private:
    client();
};

}

#endif // CLIENT_H
