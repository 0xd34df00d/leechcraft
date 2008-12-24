#ifndef URLCOMPLETIONMODEL_H
#define URLCOMPLETIONMODEL_H
#include <vector>
#include <QAbstractItemModel>
#include "historymodel.h"

class URLCompletionModel : public QAbstractItemModel
{
	Q_OBJECT

	mutable bool Valid_;
	mutable std::vector<HistoryModel::HistoryItem> Items_;
public:
	URLCompletionModel (QObject* = 0);
	virtual ~URLCompletionModel ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation,
			int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int,
			const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
public slots:
	void handleItemAdded (const HistoryModel::HistoryItem&);
private:
	void Populate () const;
};

#endif

