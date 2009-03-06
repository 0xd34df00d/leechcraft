#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QMap>
#include <interfaces/structures.h>
#include <interfaces/ifinder.h>
#include "description.h"

class Core : public QAbstractItemModel
{
	Q_OBJECT

	QObjectList Downloaders_;
	QMap<int, QString> Jobs_;
	QList<Description> Descriptions_;
	QStringList Headers_;

	static const QString OS_;

	Core ();
public:
	enum Roles
	{
		RoleDescription = 200,
		RoleContact,
		RoleTags,
		RoleLongName,
		RoleDeveloper,
		RoleAttribution,
		RoleRight,
		RoleAdult,
		RoleLanguages
	};

	static Core& Instance ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	/** Fetches the searcher from the url.
	 */
	void Add (const QString& url);
	QStringList GetCategories () const;
	IFindProxy_ptr GetProxy (const LeechCraft::Request&);
private slots:
	void handleJobFinished (int);
	void handleJobRemoved (int);
	void handleJobError (int);
private:
	void HandleEntity (const QString&);
	void HandleProvider (QObject*);
	void ReadSettings ();
	void WriteSettings ();
signals:
	void error (const QString&);
	void warning (const QString&);
	void delegateEntity (const LeechCraft::DownloadEntity&,
			int*, QObject**);
};

#endif

