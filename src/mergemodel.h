#ifndef MERGEMODEL_H
#define MERGEMODEL_H
#include <deque>
#include <QAbstractProxyModel>
#include <QStringList>

class MergeModel : public QAbstractProxyModel
{
	Q_OBJECT

	typedef std::deque<QAbstractItemModel*> models_t;
	models_t Models_;

	QStringList Headers_;
public:
	typedef models_t::iterator iterator;
	typedef models_t::const_iterator const_iterator;

	MergeModel (QObject* = 0);
	virtual ~MergeModel ();

	virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
	virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	virtual QModelIndex mapFromSource (const QModelIndex&) const;
	virtual QModelIndex mapToSource (const QModelIndex&) const;
	virtual void setSourceModel (QAbstractItemModel*);

	void AddModel (QAbstractItemModel*);
	void RemoveModel (QAbstractItemModel*);
private:
	const_iterator FindModel (const QAbstractItemModel*) const;
	iterator FindModel (const QAbstractItemModel*);
	int GetStartingRow (const_iterator) const;
	const_iterator GetModelForRow (int) const;
	iterator GetModelForRow (int);
};

#endif

