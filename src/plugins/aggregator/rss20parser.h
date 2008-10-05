#ifndef RSS20PARSER_H
#define RSS20PARSER_H
#include <QPair>
#include <QDateTime>
#include <QMap>
#include <QString>
#include "parserfactory.h"
#include "parser.h"
#include "channel.h"

class RSS20Parser : public Parser
{
    friend class ParserFactory;

    RSS20Parser ();
public:
    static RSS20Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
    virtual channels_container_t Parse (const channels_container_t&, const QDomDocument&) const;
private:
	channels_container_t Parse (const QDomDocument&) const;
    Item* ParseItem (const QDomElement&) const;
    QDateTime rfc822TimeToQDateTime (const QString&) const;
	QMap<QString, int> timezoneOffsets;
};

#endif

