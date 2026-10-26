#include <fcntl.h>
#include "utils.hpp"

// Contrainte sujet : fcntl() n'est autorisé qu'avec F_SETFL, O_NONBLOCK
// et FD_CLOEXEC. F_GETFL est donc interdit : on pose directement le flag.
bool setNonBlocking(int fd)
{
    return (fcntl(fd, F_SETFL, O_NONBLOCK) != -1);
}
