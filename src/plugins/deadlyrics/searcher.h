#ifndef SEARCHER_H
#define SEARCHER_H
#include <QObject>
#include <interfaces/ifinder.h>

class Searcher : public QObject
{
	Q_OBJECT
	
public:
	virtual ~Searcher ();
	virtual void Start (const QString&, const QString&,
			const QString& = QString ()) = 0;
	virtual void Stop () = 0;
signals:
	void textFetched (const QString&);
};

#endif

