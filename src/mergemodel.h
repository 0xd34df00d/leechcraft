#ifndef MERGEMODEL_H
#define MERGEMODEL_H
#include <deque>
#include <QAbstractProxyModel>

class MergeModel : public QAbstractProxyModel
{
	Q_OBJECT

	typedef std::deque<QAbstractItemModel*> models_t;
	models_t Models_;
public:
	typedef models_t::iterator iterator;
	typedef models_t::const_iterator const_iterator;

	MergeModel (QObject* = 0);
	virtual ~MergeModel ();

	virtual QModelIndex mapFromSource (const QModelIndex&) const;
	virtual QModelIndex mapToSource (const QModelIndex&) const;
	virtual void setSourceModel (QAbstractItemModel*);

	void AddModel (QAbstractItemModel*);
	void RemoveModel (QAbstractItemModel*);
private:
	int GetStartingRow (const_iterator) const;
};

#endif

