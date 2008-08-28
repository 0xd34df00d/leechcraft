#ifndef REPRESENTATIONMODEL_H
#define REPRESENTATIONMODEL_H
#include <QSortFilterProxyModel>

class RepresentationModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	RepresentationModel (QObject* = 0);
	virtual ~RepresentationModel ();
protected:
	virtual bool filterAcceptsColumn (int, const QModelIndex&) const;
};

#endif

