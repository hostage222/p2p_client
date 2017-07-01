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
#include <thread>

#include "p2p_client.h"

using namespace std;
using namespace p2p;

client::ptr cl = client::create();

void client_thread(string address)
{
}

int main()
{
    client::connection_result result;
    cl->connect_to_server("127.0.0.1", 14537, result);

    cout << client::to_string(result) << endl;

    return 0;
}
