#ifndef REPRESENTATIONMODEL_H
#define REPRESENTATIONMODEL_H
#include <QSortFilterProxyModel>

class RepresentationModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	RepresentationModel (QObject* = 0);
	virtual ~RepresentationModel ();
};

#endif

