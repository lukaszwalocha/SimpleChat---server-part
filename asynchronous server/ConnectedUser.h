#pragma once
#include <memory>
#include <string>
#include <boost/asio/ip/tcp.hpp>

struct ConnectedUser {
	std::shared_ptr<boost::asio::ip::tcp::socket> sessionSocket;
	std::string userIpAddress;
	std::string userId;
	std::string status;
};