#ifndef ATOMPARSER_H
#define ATOMPARSER_H
#include "parser.h"
#include "feed.h"

class QDomDocument;
class QString;

class AtomParser : public Parser
{
public:
	virtual Feed::channels_container_t Parse (const Feed::channels_container_t&,
			const QDomDocument&) const;
protected:
	virtual Feed::channels_container_t Parse (const QDomDocument&) const = 0;
	QDateTime FromRFC3339 (const QString&) const;
	virtual QString GetLink (const QDomElement&) const;
	virtual QString ParseEscapeAware (const QDomElement&) const;
};

#endif

