#ifndef SEARCHER_H
#define SEARCHER_H
#include <QObject>
#include <interfaces/ifinder.h>

struct Lyrics
{
	QString Author_;
	QString Album_;
	QString Title_;
	QString Text_;
	QString URL_;
};

bool operator== (const Lyrics&, const Lyrics&);

class Searcher : public QObject
{
	Q_OBJECT
	
public:
	virtual ~Searcher ();
	virtual void Start (const QStringList&) = 0;
	virtual void Stop (const QByteArray&) = 0;
signals:
	void textFetched (const Lyrics&);
};

#endif

