#ifndef DEADLYRICS_H
#define DEADLYRICS_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/iwantnetworkaccessmanager.h>

class DeadLyRicS : public QObject
				 , public IInfo
				 , public IFinder
				 , public IWantNetworkAccessManager
{
	Q_OBJECT
	Q_INTERFACES (IInfo IFinder IWantNetworkAccessManager)
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
	void SetNetworkAccessManager (QNetworkAccessManager*);
	QStringList GetCategories () const;
	void Start (const QString&, const QStringList&, bool);
	void Abort ();
signals:
	void entityUpdated (LeechCraft::FoundEntity);
};

#endif

