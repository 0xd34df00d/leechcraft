#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>

namespace Poco
{
    namespace Net
    {
        class HTTPServer;
        class ServerSocket;
    };
};

class Server : public QObject
{
    Q_OBJECT

    Poco::Net::HTTPServer *HTTPServer_;
    Poco::Net::ServerSocket *ServerSocket_;

    Server ();
public:
    static Server& Instance ();
    void Release ();
    bool Listen (int);
    int GetPort () const;
};

#endif

