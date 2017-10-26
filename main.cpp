#if 0
#include <memory>
#include <string>
#include "p2p_client.h"

using namespace std;
using namespace p2p;

int main()
{
    ...
    procEvents();
}

shared_ptr<p2p_client> client;

<обработчик события на открытие (разворачивание) приложения>
{
   run_server_thread();
}

<обработчик события на закрытие (сворачивание) приложения>
{
   if (client)
   {
       client->close_all_connections();
       <ожидание события DISCONNECTED и выхода из потока server_thread>
   }
}

server_thread()
{
   client = p2p_client::create();

   do
   {
       bool res = client->connect_to_server(address);
       if (!res)
       {
           <сообщение об ошибке>
       }
   }
   while (!res && <повторить?>);

   /*соответствие номера телефона уникальному идентификатору:
     address_book[phone] == friend_id;*/
   auto address_book = client->get_contacts(phones);

   while (true)
   {
       auto event = client->get_event();
       switch (event->code())
       {
       events_code::DISCONNECTED:
           <завершить поток, если сами закрываем соединение;
            попытаться переконектиться или что-нибудь типа того,
            если не хотели закрывать соединение>
           return;

       events_code::FRIEND_STATUS_UPDATED:
           auto e = static_pointer_cast<friend_status_updated_event>(event);
           auto friend_id = e->friend_id();
           bool is_active = e->is_active();
           <обновить статус контакта (в сети/не в сети)>
           break;

       events_code::FRIEND_WANTED_TO_CONNECT:
           auto e = static_pointer_cast<friend_wanted_to_connect_event>(event);
           auto friend_id = e->friend_id();
           <сообщить пользователю, что контакт хочет побеседовать>
           break;

       events_code::FRIEND_WANTED_TO_STOP_CONNECTION:
           shared_ptr<friend_wanted_to_stop_connection_event> e;
           e = static_pointer_cast<friend_wanted_to_stop_connection_event>(
                       event);
           auto friend_id = e->friend_id();
           <отобразить, что контакт расхотел разговаривать>
           break;

       events_code::FRIEND_CONFIRMED_CONNECTION:
           shared_ptr<friend_confirmed_connection_event> e;
           e = static_pointer_cast<friend_confirmed_connection_event>(event);
           auto friend_id = e->friend_id();
           <контакт согласился поболтать, возможно, как-то это отобразить>
           break;

       events_code::FRIEND_DISCARDED_CONNECTION:
           shared_ptr<friend_discarded_connection_event> e;
           e = static_pointer_cast<friend_discarded_connection_event>(event);
           auto friend_id = e->friend_id();
           <контакт не захотел разговаривать, как-то это отобразить>
           break;

       events_code::FRIEND_CONNECTED:
           auto e = static_pointer_cast<friend_connected_event>(event);
           auto friend_id = e->friend_id();
           <связь с контактом установлена, открыть новый чат>
           break;

       events_code::FRIEND_WANTED_TO_DISCONNECT:
           auto e = static_pointer_cast<friend_wanted_to_disconnect>(event);
           auto friend_id = e->friend_id();
           <можно отобразить, что контакт сам закрыл соединение, а не
            произошла какая-либо ошибка>
           break;

       events_code::FRIEND_DISCONNECTED:
           auto e = static_pointer_cast<friend_disconnected_event>(event);
           auto friend_id = e->friend_id();
           <связь с контактом разорвана, закрыть чат>
           break;

       events_code::FRIEND_NEW_MESSAGE:
           auto e = static_pointer_cast<friend_new_message_event>(event);
           auto friend_id = e->friend_id();
           string text = e->message();
           <отобразить сообщение от контакта>
           break;

      events_code::FRIEND_MESSAGE_DELIVERED:
           auto e = static_pointer_cast<friend_message_delivered_event>(event);
           auto friend_id = e->friend_id();
           message_id_type = e->message_id();
           <отобразить, что сообщение доставлено>
           break;
      }
   }
}

<обработчик события на попытку начать диалог с контактом>
{
   client->connect_to_client(friend_id);
}

<обработчик события на принятие соединения с контактом>
{
    client->confirm_connection(friend_id);
}

<обработчик события на отказ от принятия соединения с контактом>
{
    client->discard_connection(friend_id);
}

<обработчик события на закрытие соединения пользователем (или отказ от
 соединения до установки соединения)>
{
   client->close_client_connection(friend_id);
}

<обработчик события на ввод нового сообщения пользователем>
{
   message_id = client->send_message(friend_id, message);
}
#endif

#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <boost/algorithm/string.hpp>

#include "p2p_client.h"

using namespace std;
using namespace p2p;

namespace
{

client::ptr cl = client::create();
atomic<bool> run{true};
atomic<bool> ready{false};

queue<string> commands;
mutex command_mutex;
condition_variable command_cv;

bool has_cmd = false;
mutex has_cmd_mutex;
condition_variable has_cmd_cv;

bool process_server_cmd(string cmd);
void process_client_cmd(string cmd);

string get_command(bool &empty)
{
    unique_lock<mutex> locker{command_mutex};
    command_cv.wait(locker, [](){ return !commands.empty() || !run; });
    if (!run)
    {
        return "";
    }

    string res = commands.front();
    commands.pop();
    empty = commands.empty();
    return res;
}

bool read_command()
{
    {
        unique_lock<mutex> locker{has_cmd_mutex};
        has_cmd_cv.wait(locker, [](){ return !has_cmd; });
    }

    {
        lock_guard<mutex> locker{command_mutex};
        if (!commands.empty())
        {
            return true;
        }
    }

    string cmd;
    do
    {
        cout << ">";
        getline(cin, cmd);
    }
    while (cmd.empty());

    {
        lock_guard<mutex> locker{has_cmd_mutex};
        has_cmd = true;
    }

    if (cmd == "exit" || cmd == "quit")
    {
        return false;
    }
    lock_guard<mutex> locker{command_mutex};
    commands.push(cmd);
    command_cv.notify_one();
    return true;
}

void process_server()
{
    while (run)
    {
        bool commands_queue_empty;
        string cmd = get_command(commands_queue_empty);
        if (!run)
        {
            break;
        }

        bool proc = process_server_cmd(move(cmd));

        if (proc)
        {
            ready = true;
        }

        if (ready || commands_queue_empty)
        {
            lock_guard<mutex> locker{has_cmd_mutex};
            has_cmd = false;
            has_cmd_cv.notify_one();
        }

        if (proc)
        {
            //proc_events();
        }
    }
}

struct invalid_cmd
{
    invalid_cmd(string message) : message{move(message)} {}
    string text() const { return message; }
private:
    string message;
};

vector<string> get_words(string cmd)
{
    vector<string> strs;
    boost::split(strs, cmd, boost::is_any_of(" \t"));
    if (strs.empty())
    {
        throw invalid_cmd{"EMPTY COMMAND"};
    }
    return strs;
}

void check_size(const vector<string> &strs, size_t expected_size)
{
    if (strs.size() != expected_size)
    {
        throw invalid_cmd{"UNEXPECTED PARAMETERS COUNT"};
    }
}

bool process_server_cmd(string cmd)
{
    try
    {
        vector<string> strs = get_words(move(cmd));
        if (strs.size() == 1 && strs[0] == "process")
        {
            return true;
        }

        if (strs[0] == "connect")
        {
            check_size(strs, 3);
            client::connection_result result;
            bool res = cl->connect_to_server(strs[1], stoi(strs[2]), result);
            cout << "result = " << boolalpha << res << "; " <<
                    client::to_string(result) << endl;
        }
        else if (strs[0] == "register")
        {
            check_size(strs, 3);
            auto result = cl->register_on_server(strs[1], strs[2], "");
            cout << "result = " << client::to_string(result) << endl;
        }
        else if (strs[0] == "unregister")
        {
            check_size(strs, 3);
            auto result = cl->unregister_on_server(strs[1], strs[2]);
            cout << "result = " << client::to_string(result) << endl;
        }
        else if (strs[0] == "autorize")
        {
            check_size(strs, 3);
            auto result = cl->autorize_on_server(strs[1], strs[2]);
            cout << "result = " << client::to_string(result) << endl;
        }
        else
        {
            cout << "UNEXPECTED COMMAND" << endl;
        }

        return false;
    }
    catch (invalid_cmd &e)
    {
        cout << e.text() << endl;
        return false;
    }
}

void process_client_cmd(string cmd)
{
    try
    {
        vector<string> strs = get_words(move(cmd));
    }
    catch (invalid_cmd &e)
    {
        cout << e.text() << endl;
    }
}

}

int main()
{
    //cl->connect_to_server("127.0.0.1", 14537, result);

    thread server_thread{process_server};

    while (true)
    {
        if (!read_command())
        {
            break;
        }

        if (!ready)
        {
            continue;
        }

        bool empty;
        string cmd = get_command(empty);
        process_client_cmd(move(cmd));

        has_cmd = false;
    }

    run = false;
    {
        lock_guard<mutex> locker{command_mutex};
        command_cv.notify_one();
    }
    server_thread.join();

    return 0;
}
