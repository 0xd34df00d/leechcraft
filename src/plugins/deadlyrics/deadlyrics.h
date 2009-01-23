#ifndef DEADLYRICS_H
#define DEADLYRICS_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>

class DeadLyRicS : public QObject
				 , public IInfo
				 , public IFinder
{
	Q_OBJECT
	Q_INTERFACES (IInfo IFinder)
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QIcon GetIcon () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	QStringList GetCategories () const;
	void Start (const QString&, const QStringList&);
	void Abort ();
};

#endif

