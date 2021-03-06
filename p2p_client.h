/**
 * @file
 * @brief Заголовочный файл с описанием класса p2p::client
 */
#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "p2p_common.h"
#include "p2p_events.h"
#include "p2p_connection.h"

/**
 * \mainpage Index page
 *
 * see \ref p2p
 */

/**
 * @brief Пространство имён, содержащее реализацию p2p-мессенджера
 */
namespace p2p
{

/**
 * @brief Класс p2p-мессенджера
 */
class client : public std::enable_shared_from_this<client>
{
    client();

public:
    /**
     * @brief Умный указатель (с подсчётом ссылок) на объект класса
     */
    using ptr = std::shared_ptr<client>;
    /**
     * @brief Создать объект
     *
     * Конструктор класса является приватным, прямое создание запрещено
     * из-за особенностей функционирования класса std::enable_shared_from_this
     *
     * @return Указатель на созданный объект класса
     */
    static ptr create();

    /**
     * @brief Результат выполнения попытки подключения к серверу
     */
    enum class connection_result
    {
        OK,                 ///< Подключение было успешно установлено
        SHOULD_BE_UPDATE,   ///< Подключение было успешно установлено, однако
                            /// рекомендуется обновить клиент
        MUST_BE_UPDATE,     ///< Подключение не было установлено, необходимо
                            /// обновить клиент
        SERVER_OLD_VERSION, ///< Подключение было успешно установлено, однако
                            /// сервер имеет более старую версию, чем клиент
        SERVER_INCOMPATIBLE_VERSION,
                            ///< Подключение не было установлено, сервер
                            /// имеет более старую версию, несовместим с
                            /// текущей версией клиента или содержит
                            /// критические уязвимости
        NOT_CONNECTED,      ///< Не удалось подключиться к серверу
        FAILED              ///< Критическая ошибка при обмене данных с
                            /// сервером; соединение разорвано
    };
    static std::string to_string(connection_result v);
    /**
     * @brief Установить соединение с сервером; блокирующий метод
     *
     * Если соединение уже установлено, ничего не делает, возвращает true и
     * connection_result::OK; нельзя вызывать из разных потоков одновременно
     *
     * @param[in] address Адрес сервера
     * @param[in] port Порт сервера
     * @param[out] result Результат попытки подключения
     * @return Успешность подключения к серверу
     */
    bool connect_to_server(std::string address, uint16_t port,
                           connection_result &result);

    /**
     * @brief Версия программного обеспечения сервера и клиента
     *
     * Версия имеет следующий формат: "x.y.z",
     * x увеличивается при потере совместимости с предыдущими версиями или
     * при исправлении критических уязвимостей,
     * y увеличивается при добавлении нового функционала с сохранением
     * обратной совместимости,
     * z увеличивается при исправлении некритичных багов с сохранением
     * обратной совместимости;
     * если значения "x" клиента и сервера не совпадают,
     * связь между ними не может быть установлена
     */
    using version = std::string;
    /**
     * @brief Получить версию клиента
     * @return Версия клиента
     */
    version get_version();
    /**
     * @brief Получить версию сервера, к которому подключен клиент
     * @return Версия сервера, к которому подключен клиент;
     * если связь с сервером не установлена или была потеряна,
     * метод возвращает пустую строку
     */
    version get_server_version();

    /**
     * @brief Результат выполнения попытки регистрации на сервере
     */
    enum class register_result
    {
        OK,             ///< Пользователь зарегестрирован
        NEED_CODE,      ///< Необходим код авторизации, после получения
                        /// такого ответа, код с сервера должен быть
                        /// передан посредством смс-сообщения
        INVALID_CODE,   ///< Неверный код авторизации
        ALREADY_EXISTS, ///< Пользователь с указанным номером телефона
                        /// уже существует
        DISCONNECTED,   ///< Соединение с сервером было потеряно (или не было
                        /// установлено)
        FAILED          ///< Критическая ошибка при обмене данных с
                        /// сервером; соединение разорвано
    };
    static std::string to_string(register_result v);
    /**
     * @brief Зарегестрироваться на сервере
     *
     * Если пользователь уже авторизовался, ничего не делает
     *
     * @param[in] phone Номер телефона
     * @param[in] password Пароль
     * @param[in] code Код авторизации, может быть пустым,
     *                 если это первый вызов данного метода, либо если
     *                 сервер не поддерживает данный функционал
     * @return Результат выполнения попытки регистрации на сервере
     */
    register_result register_on_server(std::string phone,
                                       std::string password,
                                       std::string code);

    /**
     * @brief Результат выполнения попытки удаления регистрации с сервера
     */
    enum class unregister_result
    {
        OK,               ///< Регистрация удалена
        INVALID_ACTION,   ///< Пользователь авторизован,
                          /// удаление регистрации пользователя невозможно
        INVALID_PHONE,    ///< Номер телефона не найден
        INVALID_PASSWORD, ///< Неверный пароль
        DISCONNECTED,     ///< Соединение с сервером было потеряно (или не было
                          /// установлено)
        FAILED            ///< Критическая ошибка при обмене данных с
                          /// сервером; соединение разорвано
    };
    static std::string to_string(unregister_result v);
    /**
     * @brief Удалить регистрацию на сервере
     *
     * Если соединение не было установлено,
     * либо пользователь уже авторизовался,
     * ничего не делает
     *
     * @param[in] phone Номер телефона
     * @param[in] password Пароль
     * @return Результат выполнения попытки удаления регистрации с сервера
     */
    unregister_result unregister_on_server(std::string phone,
                                           std::string password);

    /**
     * @brief Результат выполнения авторизации на сервере
     */
    enum class autorize_result
    {
        OK,               ///< Авторизация выполнена успешно
        INVALID_ACTION,   ///< Пользователь уже авторизован,
                          /// повторная авторизация невозможна
        INVALID_PHONE,    ///< Номер телефона не найден
        INVALID_PASSWORD, ///< Неверный пароль
        DISCONNECTED,     ///< Соединение с сервером было потеряно (или не было
                          /// установлено)
        FAILED            ///< Критическая ошибка при обмене данных с
                          /// сервером; соединение разорвано
    };
    static std::string to_string(autorize_result v);
    /**
     * @brief Авторизация на сервере
     * @param[in] phone Номер телефона
     * @param[in] password Пароль
     * @return Результат выполнения авторизации на сервере
     */
    autorize_result autorize_on_server(std::string phone, std::string password);

    /**
     * @brief Закрытие соединение с сервером и всеми контактами;
     * неблокирующий метод
     *
     * Если соединение не было устанолвено, ничего не делает;
     * если соединение было установлено, закрывает его и генерирует событие
     * disconnected_event
     */
    void close_all_connections();

    /**
     * @brief Список телефонов
     */
    using phones_list = std::vector<std::string>;
    /**
     * @brief Ассоциативный массив, ставящий в соответствие уникальный
     * идентификатор контакта его номеру телефона
     *
     * Ключ - телефон, значение - уникальный идентификатор
     */
    using contacts_dictionary = std::map<std::string, friend_id_type>;
    /**
     * @brief Запросить уникальные идентификаторы пользователей по их номерам
     * телефона; метод блокирующий
     *
     * @param[in] phones Список телефонов
     * @return Ассоциативный массив, содержащий в качестве ключей номера
     * телефонов контактов, а в качестве значений уникальные идентификаторы
     * контактов; результат включает в себя только тех ползователей, которые
     * были зарегистрированы на сервере; если соединение с сервером не было
     * установлено или было разорвано во время выполнения метода,
     * либо если клиент не авторизован, возвращаемое
     * значение будет пустым
     */
    contacts_dictionary get_contacts(const phones_list &phones);

    /**
     * @brief Запросить необработанное событие; метод блокирующий
     *
     * Если очередь сообщений пуста, а соединение с сервером не установлено,
     * будет возвращено событие disconnected_event
     *
     * @return Указатель на событие
     */
    event::ptr get_event();

    /**
     * @brief Запросить соединение с контактом; неблокирующий метод
     *
     * Если соединение с выбранным контактом уже запрошено, либо уже
     * установлено, или связь с сервером не была установлена,
     * либо если клиент не авторизован, метод ничего
     * не делает; связанные события: friend_confirmed_connection_event,
     * friend_discarded_connection_event, friend_connected_event,
     * friend_disconnected_event
     *
     * @param[in] friend_id Уникальный идентификатор контакта
     */
    void connect_to_client(friend_id_type friend_id);

    /**
     * @brief Согласиться принять соденинение с контактом; неблокирующий метод
     *
     * Если запрос на соединение от указанного контакта не был ранее получен,
     * либо соединение уже установлено,
     * метод ничего не делает; связанные события:
     * friend_connected_event, friend_disconnected_event
     *
     * @param[in] friend_id Уникальный идентификатор контакта
     */
    void confirm_connection(friend_id_type friend_id);

    /**
     * @brief Отказаться принимать соединение с контактом; неблокирующий метод
     *
     * Если запрос на соединение от указанного контакта не был ранее получен,
     * либо соединение уже установлено, метод ничего не делает
     *
     * @param[in] friend_id Уникальный идентификатор контакта
     */
    void discard_connection(friend_id_type friend_id);

    /**
     * @brief Закрыть соединение с контактом или отказаться от соединения;
     * неблокирующий метод
     *
     * Если метод был вызван после вызова client::connect_to_client,
     * но до установки соединения с контактом, указанному контакту
     * будет передано сообщение об отказе установки соединения;
     * если метод был вызван после устанавки соединения,
     * соединение будет закрыто; если связь с клиентом не была установлена,
     * метод ничего не делает; связанное событие - friend_disconnected_event
     *
     * @param[in] friend_id Уникальный идентификатор контакта
     */
    void close_client_connection(friend_id_type friend_id);

    /**
     * @brief Послать сообщение контакту
     *
     * После того как сообщение будет доставлено, генерируется событие
     * friend_message_delivered_event; сообщение может быть недоставлено
     * только если потеряна связь с контактом;
     * если связь с указанным контактом не было установлена, метод ничего
     * не делает
     *
     * @param[in] friend_id Уникальный идентификатор контакта
     * @param[in] message Текст сообщения
     * @return Уникальный (для текущего объекта клиента) идентификатор
     * сообщения
     */
    message_id_type send_message(friend_id_type friend_id,
                                 const std::string &message);

    /**
     * @brief Указать, что сообщение прочитано
     *
     * После вызова метода собеседнику придёт событие
     * friend_message_readed_event
     *
     * @param[in] friend_id Уникальный идентификатор контакта
     * @param[in] message_id Уникальный (для текущего объекта клиента)
     * идентификатор сообщения
     */
    void confirm_reading(friend_id_type friend_id, message_id_type message_id);

private:
    connection::ptr con;
    version client_version;
    version server_version;

    template <typename Dict>
    auto account_operation(const Dict &answer_dict,
                           std::unique_ptr<request> req,
                           std::shared_ptr<std::string> answer)
    {
        con->send_request(move(req));
        try
        {
            con->wait_answer();
        }
        catch (connection::communication_exception&)
        {
            return Dict::mapped_type::FAILED;
        }
        catch (connection::disconnected_exception&)
        {
            return Dict::mapped_type::DISCONNECTED;
        }

        try
        {
            return answer_dict.at(*answer);
        }
        catch (std::out_of_range&)
        {
            return Dict::mapped_type::FAILED;
        }
    }
};

}

#endif // CLIENT_H
