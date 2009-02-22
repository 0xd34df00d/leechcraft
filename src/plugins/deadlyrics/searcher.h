#ifndef SEARCHER_H
#define SEARCHER_H
#include <vector>
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

typedef std::vector<Lyrics> lyrics_t;

class Searcher : public QObject
{
	Q_OBJECT
public:
	virtual ~Searcher ();
	virtual QByteArray Start (const QStringList&) = 0;
	virtual void Stop (const QByteArray&) = 0;
signals:
	void textFetched (const Lyrics&, const QByteArray&);
};

typedef boost::shared_ptr<Searcher> searcher_ptr;
typedef std::vector<searcher_ptr> searchers_t;

#endif

