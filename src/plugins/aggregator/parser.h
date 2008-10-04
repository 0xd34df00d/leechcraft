#ifndef PARSER_H
#define PARSER_H
#include <QDomDocument>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "feed.h"

struct Channel;

class Parser
{
public:
    virtual bool CouldParse (const QDomDocument&) const = 0;
    virtual Feed::channels_container_t Parse (const Feed::channels_container_t&,
			const QDomDocument&) const = 0;
	Feed::channels_container_t Parse (const Feed::channels_container_t&,
			const QByteArray&);
protected:
    QString UnescapeHTML (const QString&) const;
	QStringList GetDCCategories (const QDomElement&) const;
};

#endif

