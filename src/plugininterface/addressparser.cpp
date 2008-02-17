#include <QtDebug>
#include <QTextCodec>
#include "addressparser.h"

/*! @brief Constructor.
 *
 * Constructs the AddressParser and parses URL specified by passed string.
 *
 * @param[in] str URL to parse.
 * @sa Reparse
 */
AddressParser::AddressParser (const QString& str)
{
    Reparse (str);
}

/*! @brief Parses URL.
 *
 * Parses the new specicifed URL and sets internal data structures according to it.
 * @param[in] str URL to parse.
 */
void AddressParser::Reparse (const QString& str)
{
    QUrl url;

    url.setUrl (str);

    Protocol_ = url.scheme ();

    if (Protocol_.toLower () == "http")
        Port_ = url.port (80);
    else if (Protocol_.toLower () == "ftp")
        Port_ = url.port (21);
 else
  Port_ = url.port ();
    
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

/*! @brief Returns protocol (schema).
 *
 * If protocol isn't specified in the URL it returns just an emtpy string.
 *
 * @return Protocol specified in the URL.
 */
QString AddressParser::GetProtocol () const
{
    return Protocol_;
}

/*! @brief Returns host.
 *
 * @return Host name or IP, as specified in given URL.
 */
QString AddressParser::GetHost () const
{
    return Host_;
}

/*! @brief Returns port.
 *
 * @return Port of the URL or default for the given schema.
 */
int AddressParser::GetPort () const
{
    return Port_;
}

/*! @brief Returns login.
 *
 * @return Remote login or empty string if not specified.
 */
QString AddressParser::GetLogin () const
{
    return Login_;
}

/*! @brief Returns password.
 *
 * @return Remote password or empty string if not specified.
 */
QString AddressParser::GetPassword () const
{
    return Password_;
}

/*! @brief Returns path.
 *
 * @return Remote path.
 */
QString AddressParser::GetPath () const
{
    return Path_;
}

/*! @brief Returns query.
 *
 * @return Query without the '?' sign.
 */
QString AddressParser::GetQuery () const
{
    return Query_;
}

