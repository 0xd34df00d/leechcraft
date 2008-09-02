#ifndef FILTERMODEL_H
#define FILTERMODEL_H
#include <QSortFilterProxyModel>

class FilterModel : public QSortFilterProxyModel
{
	Q_OBJECT

	bool NormalMode_;
public:
	FilterModel (QObject *parent = 0);
	void SetTagsMode (bool);
protected:
	virtual bool filterAcceptsRow (int, const QModelIndex&) const;
};

#endif

