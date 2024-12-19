#include "../include/ZmqTools.h"
#include <iostream>

std::string ZmqRequest(zmq::socket_t& s, const std::string& r)
{
    // Преобразуем строку в сообщение ZeroMQ
    zmq::message_t request(r.c_str(), r.length()+1);

    // Отправляем запрос на сервер
    s.send(request, zmq::send_flags::none);

    // Ожидаем ответ от сервера
    zmq::message_t reply;
    s.recv(reply);

    // Преобразуем полученное сообщение обратно в строку
    std::string replyStr(reinterpret_cast<char*>(reply.data()), reply.size());

    return replyStr;
}
