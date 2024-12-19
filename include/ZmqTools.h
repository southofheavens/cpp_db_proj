#ifndef ZMQ_TOOLS_H
#define ZMQ_TOOLS_H

#include <zmq.hpp>

std::string ZmqRequest(zmq::socket_t&, const std::string&);

#endif