#pragma once
#include "ConnectedUser.h"
#include "pch.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <memory>


class Server {
public:
	Server();
	~Server() = default;
public:
	std::string readData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

	void removeDisconnectedUser();

	void writeAnswer(ConnectedUser& connectedUser, std::string& message);

	void printMessageFromClient(std::string& msg);

	void executeOperation(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

	void startClientThread(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

	void emplaceNewUser(ConnectedUser& newUser);

	void startMainEventLoop();

private:
	std::shared_ptr<boost::asio::io_context> ioContext;
	std::shared_ptr<boost::asio::ip::tcp::endpoint> endPoint;
	std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
	std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> connectedSockets;
	std::vector<ConnectedUser> connectedUsers;
	std::vector<std::string> disconnectedUserIps;
	boost::mutex disconnectedRemovalMutex;
	int connectedUsersAmount;
};