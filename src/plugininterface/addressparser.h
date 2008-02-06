#ifndef ADDRESSPARSER_H
#define ADDRESSPARSER_H
#include <QString>
#include <QUrl>

class AddressParser : public QObject
{
    QString Protocol_;
    QString Host_;
    int Port_;
    QString Login_;
    QString Password_;
    QString Path_;
    QString Query_;
public:
    AddressParser (const QString& = QString ());
    void Reparse (const QString&);

    QString GetProtocol () const;
    QString GetHost () const;
    int GetPort () const;
    QString GetLogin () const;
    QString GetPassword () const;
    QString GetPath () const;
    QString GetQuery () const;

    void SetProtocol (const QString&);
    void SetHost (const QString&);
    void SetPort (const int&);
    void SetLogin (const QString&);
    void SetPassword (const QString&);
    void SetPath (const QString&);
    void SetQuery (const QString&);
};

#endif

