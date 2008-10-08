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
	static const QString DC_;
	static const QString WFW_;
	static const QString Atom_;
	static const QString RDF_;

	QString GetLink (const QDomElement&) const;
	QString GetAuthor (const QDomElement&) const;
	QString GetCommentsRSS (const QDomElement&) const;
	QDateTime GetDCDateTime (const QDomElement&) const;
	QStringList GetAllCategories (const QDomElement&) const;
	QStringList GetDCCategories (const QDomElement&) const;
	QStringList GetPlainCategories (const QDomElement&) const;

	QDateTime FromRFC3339 (const QString&) const;
    QString UnescapeHTML (const QString&) const;
};

#endif

