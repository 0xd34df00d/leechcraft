#ifndef ITEMSFILTERMODEL_H
#define ITEMSFILTERMODEL_H
#include <QSortFilterProxyModel>
#include <QStringList>

class ItemsFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT

	bool HideRead_;
	QStringList ItemCategories_;
public:
	ItemsFilterModel (QObject* = 0);
	virtual ~ItemsFilterModel ();

	void SetHideRead (bool);
protected:
	virtual bool filterAcceptsRow (int, const QModelIndex&) const;
public slots:
	void categorySelectionChanged (const QStringList&);
};

#endif

