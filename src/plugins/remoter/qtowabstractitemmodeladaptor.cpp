#include "qtowabstractitemmodeladaptor.h"
#include <WDate>
#include <QDate>
#include <QUrl>
#include <QtDebug>
#include <plugininterface/treeitem.h>
#include "util.h"

QToWAbstractItemModelAdaptor::QToWAbstractItemModelAdaptor (QAbstractItemModel *model,
		WObject *parent)
: Wt::WAbstractItemModel (parent)
, Model_ (model)
, Root_ (new TreeItem (QVariantList ()))
{
	Indexes_ [Root_] = Wt::WModelIndex ();

	connect (Model_,
			SIGNAL (columnsAboutToBeInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsAboutToBeInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (columnsAboutToBeRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsAboutToBeRemoved (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (columnsInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (columnsRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reColumnsRemoved (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (reDataChanged (const QModelIndex&, const QModelIndex&)));
	connect (Model_,
			SIGNAL (headerDataChanged (Qt::Orientation, int, int)),
			this,
			SLOT (reHeaderDataChanged (Qt::Orientation, int, int)));
	connect (Model_,
			SIGNAL (invalidate ()),
			this,
			SLOT (reInvalidate ()));
	connect (Model_,
			SIGNAL (layoutAboutToBeChanged ()),
			this,
			SLOT (reLayoutAboutToBeChanged ()));
	connect (Model_,
			SIGNAL (layoutChanged ()),
			this,
			SLOT (reLayoutChanged ()));
	connect (Model_,
			SIGNAL (modelAboutToBeReset ()),
			this,
			SLOT (reModelAboutToBeReset ()));
	connect (Model_,
			SIGNAL (modelReset ()),
			this,
			SLOT (reModelReset ()));
	connect (Model_,
			SIGNAL (rowsAboutToBeInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsAboutToBeInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (rowsAboutToBeRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsAboutToBeRemoved (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (rowsInserted (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsInserted (const QModelIndex&, int, int)));
	connect (Model_,
			SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (reRowsRemoved (const QModelIndex&, int, int)));
}

QToWAbstractItemModelAdaptor::~QToWAbstractItemModelAdaptor ()
{
}

int QToWAbstractItemModelAdaptor::columnCount (const Wt::WModelIndex& parent) const
{
	return Model_->columnCount (Convert (parent));
}

int QToWAbstractItemModelAdaptor::rowCount (const Wt::WModelIndex& parent) const
{
	return Model_->rowCount (Convert (parent));
}

int QToWAbstractItemModelAdaptor::flags (const Wt::WModelIndex& index) const
{
	Qt::ItemFlags flags = Model_->flags (Convert (index));
	Wt::ItemFlag result;
	if (flags & Qt::ItemIsSelectable)
		result = static_cast<Wt::ItemFlag> (result | Wt::ItemIsSelectable);
	if (flags & Qt::ItemIsEditable)
		result = static_cast<Wt::ItemFlag> (result | Wt::ItemIsEditable);
	if (flags & Qt::ItemIsUserCheckable)
		result = static_cast<Wt::ItemFlag> (result | Wt::ItemIsUserCheckable);
	return result;
}

bool QToWAbstractItemModelAdaptor::hasChildren (const Wt::WModelIndex& index) const
{
	return Model_->hasChildren (Convert (index));
}

Wt::WModelIndex QToWAbstractItemModelAdaptor::parent (const Wt::WModelIndex& i) const
{
	return Indexes_ [static_cast<TreeItem*> (i.internalPointer ())->Parent ()];
}

boost::any QToWAbstractItemModelAdaptor::data (const Wt::WModelIndex& i, int role) const
{
	Qt::ItemDataRole idr;
	switch (static_cast<Wt::ItemDataRole> (role))
	{
		case Wt::DisplayRole:
			idr = Qt::DisplayRole;
			break;
		case Wt::EditRole:
			idr = Qt::EditRole;
			break;
		case Wt::CheckStateRole:
			idr = Qt::CheckStateRole;
			break;
		case Wt::ToolTipRole:
			idr = Qt::ToolTipRole;
			break;
		// TODO implement on-the-fly icon rewriting.
		default:
			return boost::any ();
	}

	return Util::Convert (Model_->data (Convert (i), idr));
}

boost::any QToWAbstractItemModelAdaptor::headerData (int section,
		Wt::Orientation orient, int role) const
{
	Qt::ItemDataRole idr;
	switch (static_cast<Wt::ItemDataRole> (role))
	{
		case Wt::DisplayRole:
			idr = Qt::DisplayRole;
			break;
		case Wt::EditRole:
			idr = Qt::EditRole;
			break;
		case Wt::CheckStateRole:
			idr = Qt::CheckStateRole;
			break;
		case Wt::ToolTipRole:
			idr = Qt::ToolTipRole;
			break;
		// TODO implement on-the-fly icon rewriting.
		default:
			return boost::any ();
	}

	return Util::Convert (Model_->headerData (section,
				(orient == Wt::Horizontal ? Qt::Horizontal : Qt::Vertical),
				idr));
}

Wt::WModelIndex QToWAbstractItemModelAdaptor::index (int row, int column,
		const Wt::WModelIndex& pi) const
{
	TreeItem *parent;
	if (pi.isValid ())
		parent = static_cast<TreeItem*> (pi.internalPointer ());
	else
		parent = Root_;

	TreeItem *item = new TreeItem (QVariantList (), parent);
	parent->AppendChild (item);

	Wt::WModelIndex result = createIndex (row, column, item);
	Indexes_ [item] = result;
	return result;
}

void QToWAbstractItemModelAdaptor::reColumnsAboutToBeInserted (const QModelIndex& index,
		int from, int to)
{
	columnsAboutToBeInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reColumnsAboutToBeRemoved (const QModelIndex& index,
		int from, int to)
{
	columnsAboutToBeRemoved (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reColumnsInserted (const QModelIndex& index,
		int from, int to)
{
	columnsInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reColumnsRemoved (const QModelIndex& index,
		int from, int to)
{
	columnsRemoved (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reDataChanged (const QModelIndex& from,
		const QModelIndex& to)
{
	dataChanged (Convert (from), Convert (to));
}

void QToWAbstractItemModelAdaptor::reHeaderDataChanged (Qt::Orientation orient,
		int from, int to)
{
	headerDataChanged ((orient == Qt::Horizontal ? Wt::Horizontal : Wt::Vertical),
			from, to);
}

void QToWAbstractItemModelAdaptor::reInvalidate ()
{
	dataChanged (index (0, 0), index (rowCount (), columnCount ()));
}

void QToWAbstractItemModelAdaptor::reLayoutAboutToBeChanged ()
{
	layoutAboutToBeChanged ();
}

void QToWAbstractItemModelAdaptor::reLayoutChanged ()
{
	layoutChanged ();
}

void QToWAbstractItemModelAdaptor::reModelAboutToBeReset ()
{
	int frow = rowCount () - 1;
	rowsAboutToBeRemoved (Wt::WModelIndex (), 0, frow);
	rowsRemoved (Wt::WModelIndex (), 0, frow);
//	No corresponding signal, let's emulate
//	modelAboutToBeReset ();
}

void QToWAbstractItemModelAdaptor::reModelReset ()
{
//	modelReset ();
}

void QToWAbstractItemModelAdaptor::reRowsAboutToBeInserted (const QModelIndex& index,
		int from, int to)
{
	rowsAboutToBeInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reRowsAboutToBeRemoved (const QModelIndex& index,
		int from, int to)
{
	rowsAboutToBeRemoved (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reRowsInserted (const QModelIndex& index,
		int from, int to)
{
	rowsInserted (Convert (index), from, to);
}

void QToWAbstractItemModelAdaptor::reRowsRemoved (const QModelIndex& index,
		int from, int to)
{
	rowsRemoved (Convert (index), from, to);
}

QModelIndex QToWAbstractItemModelAdaptor::Convert (const Wt::WModelIndex& wmi) const
{
	if (!wmi.isValid ())
		return QModelIndex ();

	return Model_->index (wmi.row (), wmi.column (), Convert (wmi.parent ()));
}

Wt::WModelIndex QToWAbstractItemModelAdaptor::Convert (const QModelIndex& qmi) const
{
	if (!qmi.isValid ())
		return Wt::WModelIndex ();

	return index (qmi.row (), qmi.column (), Convert (qmi.parent ()));
}

