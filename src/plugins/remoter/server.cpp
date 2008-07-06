#include <QStringList>
#include <QTcpSocket>
#include <QUrl>
#include "server.h"
#include "core.h"

Server::Server ()
{
}

Server& Server::Instance ()
{
    static Server inst;
    return inst;
}

void Server::Release ()
{
}

bool Server::Listen (int port)
{
}

int Server::GetPort () const
{
}

