#ifndef ADDRESSPARSER_H
#define ADDRESSPARSER_H
#include <QString>
#include <QUrl>

/*! @brief Parses URLs.
 *
 * Provides a useful ability to parse URLs and split them to parts.
 */
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
};

#endif

