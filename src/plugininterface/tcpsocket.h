#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <QTcpSocket>

class AddressParser;

class TcpSocket : public QTcpSocket
{
    Q_OBJECT

    int DefaultTimeout_;
protected:
    AddressParser *AP_;
public:
    TcpSocket ();
    virtual ~TcpSocket ();

    void Connect (const QString&, int);
    void Disconnect ();

    void Write (const QString&, bool buffer = true);
    void Write (const QByteArray&, bool buffer = true);
    void Flush ();
    QByteArray ReadLine ();
    QByteArray ReadAll ();

    void SetDefaultTimeout (int);
    int GetDefaultTimeout () const;

    void SetURL (const QString&);
    const AddressParser* GetAddressParser () const;
    static AddressParser* GetAddressParser (const QString&);
private:
    void ThrowException () const;
};

#endif

