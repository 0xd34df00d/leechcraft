#ifndef ITEMSFILTERMODEL_H
#define ITEMSFILTERMODEL_H
#include <QSortFilterProxyModel>

class ItemsFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT

	bool HideRead_;
public:
	ItemsFilterModel (QObject* = 0);
	virtual ~ItemsFilterModel ();

	void SetHideRead (bool);
protected:
	virtual bool filterAcceptsRow (int, const QModelIndex&) const;
};

#endif

