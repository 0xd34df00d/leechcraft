#ifndef SEARCHHANDLER_H
#define SEARCHHANDLER_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>
#include "description.h"

/** This class performs search on a single category with a single search
 * provider.
 */
class SearchHandler : public QAbstractItemModel
{
	Q_OBJECT
	
	Description D_;

	QString SearchString_;
	struct Result
	{
		enum Type
		{
			TypeRSS,
			TypeAtom,
			TypeHTML
		};

		Type Type_;
		int TotalResults_;
		int StartIndex_;
		int ItemsPerPage_;
		QString Response_;
		QString Filename_;
	};

	QList<Result> Results_;
	QMap<int, Result> Jobs_;
	QList<QObject*> Downloaders_;
public:
	SearchHandler (const Description&);

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	void Start (const LeechCraft::Request&);
private slots:
	void handleJobFinished (int);
	void handleJobError (int);
private:
	void HandleProvider (QObject*);
signals:
	void delegateEntity (const LeechCraft::DownloadEntity&,
			int*, QObject**);
	void error (const QString&);
	void warning (const QString&);
};

typedef boost::shared_ptr<SearchHandler> SearchHandler_ptr;

#endif

