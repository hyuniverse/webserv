#pragma once
#include <set>
#include "../GlobalConfig/GlobalConfig.hpp"
#include "../Demultiplexer/KqueueDemultiplexer.hpp"
#include "../TimeoutHandler/TimeoutHandler.hpp"
#include "../EventHandler/EventHandler.hpp"
#include "../ClientManager/ClientManager.hpp"
#include "../include/commonEnums.hpp"

class ServerManager {
	public:
		~ServerManager();

		void			setupListeningSockets();
		void			run();
		bool			isServerRunning();
		void			print();
		
		private:
		std::set<int>	listenFds_;
		// bool			listenFdStatus_; //signal 핸들링과 관련 있으므로 논의 필요
		
		bool			isListeningSocket(int fd);
		void			addClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler);
		void 			removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler);
		void 			cleanUpConnections(ClientManager& clientManager, eventHandler& eventHandler);
};
