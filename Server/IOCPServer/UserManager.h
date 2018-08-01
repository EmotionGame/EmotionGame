#pragma once
#include <WinSock2.h>
#include <vector>
#include <tbb/concurrent_queue.h>
typedef struct {
	SOCKET sock;
	SOCKADDR_IN sock_addr;
	tbb::concurrent_queue<char*> _userQueue;
}Client;


class UserManager {
public:
	UserManager();
	~UserManager();
	bool enterUser(SOCKET sock);
	bool exitUser(SOCKET sock);

	bool setJob(SOCKET sock);
	void* getJob(SOCKET sock);
private:
	std::vector<Client> clients;
};