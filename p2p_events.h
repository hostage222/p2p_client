#ifndef P2P_EVENTS_H
#define P2P_EVENTS_H

#include <string>
#include <memory>
#include "p2p_common.h"

namespace p2p
{

class client;

enum class events_code : event_code_type {
    DISCONNECTED,
    FRIEND_STATUS_UPDATED,
    FRIEND_WANTED_TO_CONNECT,
    FRIEND_CONNECTED,
    FRIEND_DISCONNECTED,
    FRIEND_NEW_MESSAGE
};

class event
{
public:
    virtual ~event() {}
    using ptr = std::shared_ptr<event>;

    events_code code() const noexcept { return code_value; }

protected:
    event(events_code code) noexcept :
        code_value{code}
    {
    }

private:
    const events_code code_value;
};

class disconnected_event : public event
{
public:
    disconnected_event() noexcept :
        event{events_code::DISCONNECTED}
    {
    }
};

class friend_event : public event
{
public:
    std::string friend_id() const { return friend_id_value; }

protected:
    friend_event(events_code code, std::string friend_id) noexcept :
        event{code}, friend_id_value{friend_id}
    {
    }

private:
    const std::string friend_id_value;
};

template <events_code code>
class friend_event_code_template : public friend_event
{
public:
    friend_event_code_template(std::string friend_id) noexcept :
        friend_event{code, friend_id}
    {
    }
};

class friend_status_updated_event : public friend_event
{
public:
    friend_status_updated_event(std::string friend_id,
                                bool is_active) noexcept :
        friend_event{events_code::FRIEND_STATUS_UPDATED, friend_id},
        is_active_value{is_active}
    {
    }

    bool is_active() const noexcept { return is_active_value; }

private:
    const bool is_active_value;
};

using friend_wanted_to_connect_event =
    friend_event_code_template<events_code::FRIEND_WANTED_TO_CONNECT>;
using friend_connected_event =
    friend_event_code_template<events_code::FRIEND_CONNECTED>;
using friend_disconnected_event =
    friend_event_code_template<events_code::FRIEND_DISCONNECTED>;

class friend_new_message_event : public friend_event
{
public:
    friend_new_message_event(std::string friend_id,
                             const std::string &message) noexcept :
        friend_event{events_code::FRIEND_NEW_MESSAGE, friend_id},
        message_value{message}
    {
    }
    std::string message() const { return message_value; }

private:
    const std::string message_value;
};

}

#endif // P2P_EVENTS_H
