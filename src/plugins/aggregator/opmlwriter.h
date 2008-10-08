#ifndef OPMLWRITER_H
#define OPMLWRITER_H
#include "feed.h"

class QByteArray;
class QDomElement;
class QDomDocument;
class QDomNode;
class QStringList;

class OPMLWriter
{
	QByteArray *Data_;
public:
	OPMLWriter (QByteArray*);
	~OPMLWriter ();

	void Write (const feeds_container_t&) const;
private:
	void WriteHead (QDomElement&,
			QDomDocument&) const;
	void WriteBody (QDomElement&,
			QDomDocument&,
			const feeds_container_t&) const;
	QDomElement GetElementForTags (const QStringList&,
			QDomNode&,
			QDomDocument&) const;
};

#endif

