#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>

class Server : public QObject
{
    Q_OBJECT

    Server ();
public:
    static Server& Instance ();
    void Release ();
    bool Listen (int);
    int GetPort () const;
};

#endif

