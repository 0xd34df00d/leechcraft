#ifndef SEARCHHANDLER_H
#define SEARCHHANDLER_H
#include <QAbstractItemModel>
#include "description.h"

/** This class performs search on a single category with a single search
 * provider.
 */
class SearchHandler : public QAbstractItemModel
{
	Q_OBJECT
public:
	SearchHandler ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

	void Start ();
};

#endif

