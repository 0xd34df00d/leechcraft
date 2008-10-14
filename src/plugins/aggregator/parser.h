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

	/*! @brief Parses the document
	 * 
	 * Parses the passed XML document, differs it with already existing
	 * elements passed in old, puts new elements into returned container
	 * and already existing ones - into modified parameter.
	 *
	 * @param[in] old already existing items.
	 * @param[out] modified modified items.
	 * @param[in] document byte array with XML document.
	 * @return channels_container_t with new items.
	 */
    virtual channels_container_t Parse (const channels_container_t& old,
			channels_container_t& modified,
			const QDomDocument& document) const = 0;
protected:
	static const QString DC_;
	static const QString WFW_;
	static const QString Atom_;
	static const QString RDF_;
	static const QString Slash_;

	QString GetLink (const QDomElement&) const;
	QString GetAuthor (const QDomElement&) const;
	QString GetCommentsRSS (const QDomElement&) const;
	int GetNumComments (const QDomElement&) const;
	QDateTime GetDCDateTime (const QDomElement&) const;
	QStringList GetAllCategories (const QDomElement&) const;
	QStringList GetDCCategories (const QDomElement&) const;
	QStringList GetPlainCategories (const QDomElement&) const;

	QDateTime FromRFC3339 (const QString&) const;
    QString UnescapeHTML (const QString&) const;
};

#endif

