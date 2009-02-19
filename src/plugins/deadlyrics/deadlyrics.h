#ifndef DEADLYRICS_H
#define DEADLYRICS_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iwantnetworkaccessmanager.h>

class DeadLyRicS : public QObject
				 , public IInfo
				 , public IFinder
				 , public IJobHolder
				 , public IWantNetworkAccessManager
{
	Q_OBJECT
	Q_INTERFACES (IInfo IFinder IJobHolder IWantNetworkAccessManager)
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

	QAbstractItemModel* GetRepresentation () const;
	LeechCraft::Util::HistoryModel* GetHistory () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;
	void ItemSelected (const QModelIndex&);

	QStringList GetCategories () const;
	boost::shared_ptr<IFindProxy> GetProxy (const LeechCraft::Request&);
	void Reset ();
signals:
	void entityUpdated (LeechCraft::FoundEntity);
};

#endif

