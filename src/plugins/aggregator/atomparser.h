#ifndef ATOMPARSER_H
#define ATOMPARSER_H
#include "parser.h"
#include "channel.h"

class QDomDocument;
class QString;

class AtomParser : public Parser
{
public:
	virtual channels_container_t Parse (const channels_container_t&,
			const QDomDocument&) const;
protected:
	virtual channels_container_t Parse (const QDomDocument&) const = 0;
	QDateTime FromRFC3339 (const QString&) const;
	virtual QString ParseEscapeAware (const QDomElement&) const;
};

#endif

