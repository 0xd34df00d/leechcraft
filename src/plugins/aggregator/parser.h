#ifndef PARSER_H
#define PARSER_H
#include <QDomDocument>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "channel.h"

class Parser
{
public:
    virtual bool CouldParse (const QDomDocument&) const = 0;
    virtual channels_container_t Parse (const channels_container_t&,
			const QDomDocument&) const = 0;
	channels_container_t Parse (const channels_container_t&,
			const QByteArray&);
protected:
    QString UnescapeHTML (const QString&) const;
	QStringList GetDCCategories (const QDomElement&) const;
};

#endif

