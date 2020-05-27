#include "pch.h"
#include "Server.h"


Server::Server() {

	this->ioContext = std::make_shared<boost::asio::io_context>();

	this->mainEndPoint = std::make_shared<boost::asio::ip::tcp::endpoint>(boost::asio::ip::tcp::v4(), 1233);
	this->endPoint = std::make_shared<boost::asio::ip::tcp::endpoint>(boost::asio::ip::tcp::v4(), 1234);
	this->endPoint__2 = std::make_shared<boost::asio::ip::tcp::endpoint>(boost::asio::ip::tcp::v4(), 1235);

	this->mainAcceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioContext, *mainEndPoint);
	this->acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioContext, *endPoint);
	this->acceptor__2 = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioContext, *endPoint__2);
}


std::string Server::readData(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {

	boost::asio::streambuf buffer;
	boost::asio::read_until(*socket, buffer, '\n');
	std::string dataFromSocket = boost::asio::buffer_cast<const char*>(buffer.data());

	return dataFromSocket;
}

void Server::emplaceNewUser(ConnectedUser& newUser, std::string userType) {
	

	if (userType == "gamer") {
		connectedGamers.emplace_back(newUser);
	}

	else if (userType == "angler") {
		connectedAnglers.emplace_back(newUser);
	}

	connectedUsers.emplace_back(newUser);
}

void Server::removeDisconnectedUser(std::vector<ConnectedUser>& connectedUsers) {

	connectedUsers.erase(std::remove_if(connectedUsers.begin(), connectedUsers.end(), [&](auto& user) {
		return user.status == "disconnected";
		; }), connectedUsers.end());

}


void Server::writeAnswer(ConnectedUser& connectedUser, std::string& message) {

	try {
		const std::string msg = message + '\n';
		boost::asio::write(*connectedUser.sessionSocket, boost::asio::buffer(msg));
	}
	catch (std::exception& e) {
		connectedUser.status = "disconnected";
		return;
	}
}


void Server::printMessageFromClient(std::string& msg) {

	std::cout << msg << std::endl;

}

void Server::executeOperation(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::vector<ConnectedUser>& connectedUsers) {

	while (true) {
		try {

			std::string msg = this->readData(socket);
			boost::lock_guard<boost::mutex> locker(disconnectedRemovalMutex);
			this->removeDisconnectedUser(connectedUsers);

			for (auto& connectedUser : connectedUsers) {
				try {
					writeAnswer(connectedUser, msg);
				}
				catch (std::exception& e) {
					continue;
				}
			}

			std::cout << "Connected users: " << connectedUsers.size() << std::endl;
			printMessageFromClient(msg);

			if (msg == "end\n") {
				std::cout << "Exitting.." << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return;
			}
		}
		catch (std::exception& e) {
			std::cout << "ERR!!" << std::endl;
			std::cout << e.what() << std::endl;
			break;
		}
	}
}

void Server::startClientThread(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string userType) {

	std::function<void()> execOperation;

	if (userType == "gamer") {
		execOperation = [=]() {this->executeOperation(socket, std::ref(this->connectedGamers)); };
	}
	else if (userType == "angler") {
		execOperation = [=]() {this->executeOperation(socket, std::ref(this->connectedAnglers)); };
	}

	boost::thread newClientThread{
		execOperation
	};

	if (newClientThread.joinable()) {
		newClientThread.detach();
	}
}

void Server::startNewConnectionThread() {

	while (true) {

		std::cout << "Server is listening for connections to the main room.." << std::endl;
		try {
			ConnectedUser newUser;
			newUser.sessionSocket = std::make_shared<boost::asio::ip::tcp::socket>(*this->ioContext);
			this->mainAcceptor->accept(*newUser.sessionSocket);
			newUser.userIpAddress = boost::lexical_cast<std::string>(newUser.sessionSocket->remote_endpoint());
			newUser.status = "connected";
			
			boost::thread newConnectionThread{
				[=]() {this->startConnection(newUser); }

			};

			if (newConnectionThread.joinable()) {
				newConnectionThread.detach();
			}
		}
		catch (std::exception& e) {
			std::cout << "Err accepting new user socket or thread-run related issue." << std::endl;
			break;
		}
	}

}

std::string Server::getTokenMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {

	boost::asio::streambuf tokenBuffer;
	boost::asio::read_until(*socket, tokenBuffer, '\n');

	std::cout << std::string{ boost::asio::buffer_cast<const char*>(tokenBuffer.data()) } << std::endl;

	return std::string{boost::asio::buffer_cast<const char*>(tokenBuffer.data())};
}

void Server::startConnection(ConnectedUser newUser) {
	try {
		std::string tokenMessage = getTokenMessage(newUser.sessionSocket);

		newUser.sessionSocket.reset();
		newUser.sessionSocket = std::make_shared<boost::asio::ip::tcp::socket>(*this->ioContext);
		
		if (tokenMessage == "connect to Gamers\n") {
			this->acceptor->accept(*newUser.sessionSocket);
			emplaceNewUser(newUser, "gamer");
			this->startClientThread(newUser.sessionSocket, "gamer");
		}
		else if (tokenMessage == "connect to Anglers\n") {
			this->acceptor__2->accept(*newUser.sessionSocket);
			emplaceNewUser(newUser, "angler");
			this->startClientThread(newUser.sessionSocket, "angler");
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << " in thread " << boost::this_thread::get_id << std::endl;
	}
}

