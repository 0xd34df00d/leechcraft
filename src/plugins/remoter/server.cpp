#include <QStringList>
#include <QTcpSocket>
#include <QUrl>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Application.h>
#include "server.h"
#include "reply.h"
#include "httprequesthandlerfactory.h"
#include "core.h"

class App : public Poco::Util::Application
{
public:
    App ()
    {
    }

    ~App ()
    {
    }
};

Server::Server ()
{
    App app;
    std::vector<std::string> args;
    args.push_back ("lc-remoter-exec");
    app.init (args);
}

Server& Server::Instance ()
{
    static Server inst;
    return inst;
}

void Server::Release ()
{
    HTTPServer_->stop ();
    delete HTTPServer_;
    delete ServerSocket_;
}

bool Server::Listen (int port)
{
    qDebug () << port;
    ServerSocket_ = new Poco::Net::ServerSocket (port);

    Poco::Net::HTTPServerParams *params = new Poco::Net::HTTPServerParams ();
    params->setSoftwareVersion ("LeechCraft::Remoter/0.3.0");

    HTTPServer_ = new Poco::Net::HTTPServer (new HTTPRequestHandlerFactory (), *ServerSocket_, params);
    HTTPServer_->start ();
}

int Server::GetPort () const
{
    return HTTPServer_->port ();
}

