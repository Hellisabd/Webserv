#include "Epoll.hpp"

Epoll::Epoll() {
	_epoll_fd = epoll_create(MAX_EVENTS);
	
}