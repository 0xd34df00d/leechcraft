#ifndef RSS20PARSER_H
#define RSS20PARSER_H
#include <QDateTime>
#include "rssparser.h"
#include "channel.h"

class RSS20Parser : public RSSParser
{
    RSS20Parser ();
public:
	virtual ~RSS20Parser ();
    static RSS20Parser& Instance ();
    virtual bool CouldParse (const QDomDocument&) const;
private:
	channels_container_t Parse (const QDomDocument&) const;
	Item* ParseItem (const QDomElement&) const;
};

#endif

