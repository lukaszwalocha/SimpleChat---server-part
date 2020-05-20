#include "pch.h"
#include "Server.h"


Server::Server() {

	this->ioContext = std::make_shared<boost::asio::io_context>();
	this->endPoint = std::make_shared<boost::asio::ip::tcp::endpoint>(boost::asio::ip::tcp::v4(), 1234);
	this->acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(*ioContext, *endPoint);

}


std::string Server::readData(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {

	boost::asio::streambuf buffer;
	boost::asio::read_until(*socket, buffer, '\n');
	std::string dataFromSocket = boost::asio::buffer_cast<const char*>(buffer.data());

	return dataFromSocket;

}

void Server::emplaceNewUser(ConnectedUser& newUser) {
	connectedUsers.emplace_back(newUser);
}

void Server::removeDisconnectedUser() {

	connectedUsers.erase(std::remove_if(connectedUsers.begin(), connectedUsers.end(), [&](auto& user) {
		return user.status == "disconnected";
		; }), connectedUsers.end());

}


void Server::writeAnswer(ConnectedUser& connectedUser, std::string& message) {

	try {
		const std::string msg = message + "\n";
		boost::asio::write(*connectedUser.sessionSocket, boost::asio::buffer(msg));
	}
	catch (std::exception& e) {
		connectedUser.status = "disconnected";
		std::cout << typeid(e).name() << std::endl;
		return;
	}
}


void Server::printMessageFromClient(std::string& msg) {

	std::cout << msg << std::endl;

}

void Server::executeOperation(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {

	int messagesCounter = 0; // this counter is needed to identify if the messages is not that informing that user joined the chat room
	ConnectedUser userToDisconnect;

	while (true) {
		try {

			std::string msg = this->readData(socket);
			boost::lock_guard<boost::mutex> locker(disconnectedRemovalMutex);
			this->removeDisconnectedUser();

			if (messagesCounter > 0) {

				for (auto& connectedUser : connectedUsers) {
					try {
						writeAnswer(connectedUser, msg);
					}
					catch (std::exception& e) {
						continue;
					}
				}
			}
			std::cout << "Connected sockets: " << connectedUsers.size() << std::endl;
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

		messagesCounter++;
	}
}

void Server::startClientThread(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {

	boost::thread newClientThread{
		[=]() {this->executeOperation(socket); }
	};

	if (newClientThread.joinable()) {
		newClientThread.detach();
	}
}



void Server::startMainEventLoop() {

	while (true) {

		std::cout << "Server is listening.. " << std::endl;

		ConnectedUser newUser;
		newUser.sessionSocket = std::make_shared<boost::asio::ip::tcp::socket>(*this->ioContext);
		this->acceptor->accept(*newUser.sessionSocket);
		newUser.userIpAddress = boost::lexical_cast<std::string>(newUser.sessionSocket->remote_endpoint());
		newUser.status = "connected";

		this->emplaceNewUser(newUser);

		std::cout << "Connection with: " << newUser.userIpAddress << " established!" << std::endl;
		this->startClientThread(newUser.sessionSocket);

	}
}