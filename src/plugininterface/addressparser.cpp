#include <QtDebug>
#include <QTextCodec>
#include "addressparser.h"

AddressParser::AddressParser (const QString& str)
{
    Reparse (str);
}

void AddressParser::Reparse (const QString& str)
{
    QUrl url;

    url.setUrl (str);

    Protocol_ = url.scheme ();

    if (Protocol_.toLower () == "http")
        Port_ = url.port (80);
    else
        Port_ = url.port (21);
    
    Host_ = url.host ();
    Login_ = url.userName ();
    Password_ = url.password ();
    Path_ = url.path ();
    Query_ = url.encodedQuery ();

    QString newPath;

    for (int i = 0; i < Path_.size (); ++i)
    {
        if (Path_ [i] != '%')
            newPath.append (Path_ [i]);
        else
        {
            wchar_t ch = Path_.mid (++i, 2).toInt (0, 16);
            newPath.append (ch);
            ++i;
        }
    }

    Path_ = newPath;
}

QString AddressParser::GetProtocol () const
{
    return Protocol_;
}

QString AddressParser::GetHost () const
{
    return Host_;
}

int AddressParser::GetPort () const
{
    return Port_;
}

QString AddressParser::GetLogin () const
{
    return Login_;
}

QString AddressParser::GetPassword () const
{
    return Password_;
}

QString AddressParser::GetPath () const
{
    return Path_;
}

QString AddressParser::GetQuery () const
{
    return Query_;
}

void AddressParser::SetProtocol (const QString& val)
{
    Protocol_ = val;
}

void AddressParser::SetHost (const QString& val)
{
    Host_ = val;
}

void AddressParser::SetPort (const int& val)
{
    Port_ = val;
}

void AddressParser::SetLogin (const QString& val)
{
    Login_ = val;
}

void AddressParser::SetPassword (const QString& val)
{
    Password_ = val;
}

void AddressParser::SetPath (const QString& val)
{
    Path_ = val;
}

void AddressParser::SetQuery (const QString& val)
{
    Query_ = val;
}

