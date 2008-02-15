#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>

class Server : public QTcpServer
{
    Q_OBJECT

    Server ();
public:
    static Server& Instance ();
    void Release ();
private slots:
    void ready ();
};

#endif

