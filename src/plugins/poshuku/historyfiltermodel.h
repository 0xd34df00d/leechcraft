#ifndef HISTORYFILTERMODEL_H
#define HISTORYFILTERMODEL_H
#include <QSortFilterProxyModel>

class HistoryFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	HistoryFilterModel (QObject* = 0);
protected:
	virtual bool filterAcceptsRow (int, const QModelIndex&) const;
};

#endif

