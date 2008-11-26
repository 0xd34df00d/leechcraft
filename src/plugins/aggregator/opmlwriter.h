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

	QString Write (const channels_shorts_t&,
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
			const channels_shorts_t&) const;
	QDomElement GetElementForTags (const QStringList&,
			QDomNode&,
			QDomDocument&) const;
};

#endif

