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

	void removeDisconnectedUser(std::vector<ConnectedUser>& connectedUsers);

	void writeAnswer(ConnectedUser& connectedUser, std::string& message);

	void printMessageFromClient(std::string& msg);

	void executeOperation(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::vector<ConnectedUser>& connectedUsers);

	void startClientThread(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string userType);

	void emplaceNewUser(ConnectedUser& newUser, std::string userType);

	void startConnection(ConnectedUser newUser);

	void startConnectionEventLoop();

	void startNewConnectionThread();

	std::string getTokenMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

private:
	std::shared_ptr<boost::asio::io_context> ioContext;
private:
	std::shared_ptr<boost::asio::ip::tcp::endpoint> mainEndPoint;
	std::shared_ptr<boost::asio::ip::tcp::endpoint> anteRoom;
	std::shared_ptr<boost::asio::ip::tcp::endpoint> endPoint;
	std::shared_ptr<boost::asio::ip::tcp::endpoint> endPoint__2;
private:
	std::shared_ptr<boost::asio::ip::tcp::acceptor> mainAcceptor;
	std::shared_ptr<boost::asio::ip::tcp::acceptor> anteRoomAcceptor;
	std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
	std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor__2;
private:
	std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> connectedSockets;
	std::vector<ConnectedUser> connectedUsers;
	std::vector<ConnectedUser> connectedGamers;
	std::vector<ConnectedUser> connectedAnglers;
	std::vector<std::string> disconnectedUserIps;
	boost::mutex disconnectedRemovalMutex;
	int connectedUsersAmount;
};