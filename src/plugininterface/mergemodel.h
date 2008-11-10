#ifndef MERGEMODEL_H
#define MERGEMODEL_H
#include <deque>
#include <QAbstractProxyModel>
#include <QStringList>
#include "config.h"

class LEECHCRAFT_API MergeModel : public QAbstractProxyModel
{
	Q_OBJECT

	typedef std::deque<QAbstractItemModel*> models_t;
	models_t Models_;

	QStringList Headers_;
public:
	typedef models_t::iterator iterator;
	typedef models_t::const_iterator const_iterator;

	MergeModel (const QStringList&, QObject* = 0);
	virtual ~MergeModel ();

	virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
	virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	virtual QModelIndex mapFromSource (const QModelIndex&) const;
	virtual QModelIndex mapToSource (const QModelIndex&) const;
	virtual void setSourceModel (QAbstractItemModel*);

	void AddModel (QAbstractItemModel*);
	void RemoveModel (QAbstractItemModel*);

	const_iterator FindModel (const QAbstractItemModel*) const;
	iterator FindModel (const QAbstractItemModel*);
	int GetStartingRow (const_iterator) const;
	const_iterator GetModelForRow (int) const;
	iterator GetModelForRow (int);
public Q_SLOTS:
	void handleColumnsAboutToBeInserted (const QModelIndex&, int, int);
	void handleColumnsAboutToBeRemoved (const QModelIndex&, int, int);
	void handleColumnsInserted (const QModelIndex&, int, int);
	void handleColumnsRemoved (const QModelIndex&, int, int);
	void handleDataChanged (const QModelIndex&, const QModelIndex&);
	void handleRowsAboutToBeInserted (const QModelIndex&, int, int);
	void handleRowsAboutToBeRemoved (const QModelIndex&, int, int);
	void handleRowsInserted (const QModelIndex&, int, int);
	void handleRowsRemoved (const QModelIndex&, int, int);
};

#endif

