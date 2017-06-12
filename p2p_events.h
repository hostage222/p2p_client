#ifndef P2P_EVENTS_H
#define P2P_EVENTS_H

#include <string>
#include <memory>
#include "p2p_common.h"

namespace p2p
{

class client;

enum class events_code : event_code_type {
    DISCONNECTED, ///< событие disconnected_event
    FRIEND_STATUS_UPDATED, ///< событие friend_status_updated_event
    FRIEND_WANTED_TO_CONNECT, ///< событие friend_wanted_to_connect_event
    FRIEND_WANTED_TO_STOP_CONNECTION,
    ///< событие friend_wanted_to_stop_connection_event
    FRIEND_CONFIRMED_CONNECTION, ///< событие friend_confirmed_connection_event
    FRIEND_DISCARDED_CONNECTION, ///< событие friend_discarded_connection_event
    FRIEND_CONNECTED, ///< событие friend_connected_event
    FRIEND_WANTED_TO_DISCONNECT, ///< событие friend_wanted_to_disconnect_event
    FRIEND_DISCONNECTED, ///< событие friend_disconnected_event
    FRIEND_NEW_MESSAGE, ///< событие friend_new_message_event
    FRIEND_MESSAGE_DELIVERED, ///< событие friend_message_delivered_event
    FRIEND_MESSAGE_READED, ///< событие friend_message_readed_event
};

/**
 * @brief Базовый класс событий
 */
class event
{
public:
    /**
     * @brief Виртуальный деструктор для того, чтобы работал dynamic_cast
     */
    virtual ~event() {}
    /**
     * @brief Умный указатель (с подсчётом ссылок) на событие
     */
    using ptr = std::shared_ptr<event>;

    /**
     * @brief Возвращает код события
     * @return Код события
     */
    events_code code() const noexcept { return code_value; }

protected:
    event(events_code code) noexcept :
        code_value{code}
    {
    }

private:
    const events_code code_value;
};

/**
 * @brief Закрытие соединения с сервером и всеми контактами
 */
class disconnected_event : public event
{
public:
    disconnected_event() noexcept :
        event{events_code::DISCONNECTED}
    {
    }
};

/**
 * @brief Базовый класс для всех событий, связанных с определённым контактом
 */
class friend_event : public event
{
public:
    /**
     * @brief Возвращает уникальный идентификатор контакта, с которым
     * связано событие
     * @return Уникальный идентификатор контакта
     */
    friend_id_type friend_id() const { return friend_id_value; }

protected:
    friend_event(events_code code, friend_id_type friend_id) noexcept :
        event{code}, friend_id_value{friend_id}
    {
    }

private:
    const friend_id_type friend_id_value;
};

/**
 * @brief Шаблон для объявления событий, связанных с определённым контактом,
 * не содержащих дополнительных данных; нужен только для уменьшения объёма кода
 */
template <events_code code>
class friend_event_code_template : public friend_event
{
public:
    friend_event_code_template(friend_id_type friend_id) noexcept :
        friend_event{code, friend_id}
    {
    }
};

/**
 * @brief Изменение статуса контакта (в сети или не в сети)
 *
 * Событие может прийти в любой момент после установки соединения с
 * сервером, назависимо от действий пользователя
 */
class friend_status_updated_event : public friend_event
{
public:
    friend_status_updated_event(friend_id_type friend_id,
                                bool is_active) noexcept :
        friend_event{events_code::FRIEND_STATUS_UPDATED, friend_id},
        is_active_value{is_active}
    {
    }

    /**
     * @brief Показывает, подключен ли контакт к серверу
     * @return Подключен ли контакт к серверу
     */
    bool is_active() const noexcept { return is_active_value; }

private:
    const bool is_active_value;
};

/**
 * @brief Контакт хочет установить соединение
 *
 * После получения события может быть вызван метод
 * client::confirm_connection или client::discard_connection
 */
using friend_wanted_to_connect_event =
    friend_event_code_template<events_code::FRIEND_WANTED_TO_CONNECT>;
/**
 * @brief Контакт отказался от установки соединения, которое сам же
 * инициализировал; может прийти только после события
 * friend_wanted_to_connect_event
 */
using friend_wanted_to_stop_connection_event =
    friend_event_code_template<events_code::FRIEND_WANTED_TO_STOP_CONNECTION>;
/**
 * @brief Контакт дал согласие на установку соединения; само соединение пока
 * не установлено
 */
using friend_confirmed_connection_event =
    friend_event_code_template<events_code::FRIEND_CONFIRMED_CONNECTION>;
/**
 * @brief Контакт отказался от установки соединения
 */
using friend_discarded_connection_event =
    friend_event_code_template<events_code::FRIEND_DISCARDED_CONNECTION>;
/**
 * @brief Соединение с контактом установлено
 */
using friend_connected_event =
    friend_event_code_template<events_code::FRIEND_CONNECTED>;
/**
 * @brief Контакт хочет разорвать соединение; событие приходит до события
 * friend_disconnected_event
 */
using friend_wanted_to_disconnect =
    friend_event_code_template<events_code::FRIEND_WANTED_TO_DISCONNECT>;
/**
 * @brief Соединение с контактом разорвано (причина не уточняется)
 */
using friend_disconnected_event =
    friend_event_code_template<events_code::FRIEND_DISCONNECTED>;

/**
 * @brief Пришло новое сообщение от контакта
 */
class friend_new_message_event : public friend_event
{
public:
    friend_new_message_event(friend_id_type friend_id,
                             const std::string &message) noexcept :
        friend_event{events_code::FRIEND_NEW_MESSAGE, friend_id},
        message_value{message}
    {
    }
    /**
     * @brief Возвращает текст сообщения
     * @return Текст сообщения
     */
    std::string message() const { return message_value; }

private:
    const std::string message_value;
};

/**
 * @brief Сообщение было доставлено
 */
class friend_message_delivered_event : public friend_event
{
public:
    friend_message_delivered_event(friend_id_type friend_id,
                                   message_id_type message_id) :
        friend_event{events_code::FRIEND_MESSAGE_DELIVERED, friend_id},
        message_id_value{message_id}
    {
    }
    /**
     * @brief Возвращает идентификатор доставленного сообщения
     * @return Идентификатор доставленного сообщения
     */
    message_id_type message_id() const { return message_id_value; }

private:
    const message_id_type message_id_value;
};

/**
 * @brief Сообщение было прочитано
 */
class friend_message_readed_event : public friend_event
{
public:
    friend_message_readed_event(friend_id_type friend_id,
                                message_id_type message_id) :
        friend_event{events_code::FRIEND_MESSAGE_DELIVERED, friend_id},
        message_id_value{message_id}
    {
    }
    /**
     * @brief Возвращает идентификатор прочитанного сообщения
     * @return Идентификатор прочитанного сообщения
     */
    message_id_type message_id() const { return message_id_value; }

private:
    const message_id_type message_id_value;
};

}

#endif // P2P_EVENTS_H
