#ifndef OPMLWRITER_H
#define OPMLWRITER_H
#include "feed.h"
#include <QString>

class QDomElement;
class QDomDocument;
class QDomNode;
class QStringList;

class OPMLWriter
{
public:
	OPMLWriter ();
	~OPMLWriter ();

	QString Write (const feeds_container_t&,
			const QString&,
			const QString&,
			const QString&) const;
private:
	void WriteHead (QDomElement&,
			QDomDocument&,
			const QString&,
			const QString&,
			const QString&) const;
	void WriteBody (QDomElement&,
			QDomDocument&,
			const feeds_container_t&) const;
	QDomElement GetElementForTags (const QStringList&,
			QDomNode&,
			QDomDocument&) const;
};

#endif

