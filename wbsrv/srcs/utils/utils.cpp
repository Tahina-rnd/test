#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <string>
#include <stdexcept>
#include "Socket.hpp"
#include "utils.hpp"

bool setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
}