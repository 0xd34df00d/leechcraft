#ifndef RSS20PARSER_H
#define RSS20PARSER_H
#include <QPair>
#include <QDateTime>
#include <QMap>
#include <QString>
#include "parserfactory.h"
#include "parser.h"

class Item;

class RSS20Parser : public Parser
{
    Q_OBJECT

    friend class ParserFactory;

    RSS20Parser ();
public:
    static RSS20Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
    virtual Feed::channels_container_t Parse (const Feed::channels_container_t&, const QDomDocument&) const;
private:
	Feed::channels_container_t Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
    QDateTime rfc822TimeToQDateTime (const QString&) const;
	QMap<QString, int> timezoneOffsets;
};

#endif

