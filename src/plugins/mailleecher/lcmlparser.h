#ifndef LCMLPARSER_H
#define LCMLPARSER_H
#include <QString>
#include <QList>

struct LCML
{
	QString Address_;
	QString Login_;
	QList<QByteArray> UIDLs_;
};

class LCMLParser
{
public:
	static LCML Parse (const QByteArray&);
	static QByteArray Create (const LCML&);
};

#endif

