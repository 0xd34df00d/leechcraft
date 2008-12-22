#include <QtDebug>
#include <plugininterface/treeitem.h>
#include <plugininterface/proxy.h>
#include <iterator>
#include "torrentfilesmodel.h"
#include "core.h"

using LeechCraft::Util::TreeItem;
using LeechCraft::Util::Proxy;

TorrentFilesModel::TorrentFilesModel (bool addDia, QObject *parent)
: QAbstractItemModel (parent)
, AdditionDialog_ (addDia)
, FilesInTorrent_ (0)
{
    QList<QVariant> rootData;
    if (AdditionDialog_)
        rootData << tr ("Name") << tr ("Size");
    else
        rootData << tr ("Name") << tr ("Size") << tr ("Priority") << tr ("Progress");
    RootItem_ = new TreeItem (rootData);
}

TorrentFilesModel::~TorrentFilesModel ()
{
    delete RootItem_;
}

int TorrentFilesModel::columnCount (const QModelIndex& index) const
{
    if (index.isValid ())
        return static_cast<TreeItem*> (index.internalPointer ())->ColumnCount ();
    else
        return RootItem_->ColumnCount ();
}

QVariant TorrentFilesModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (AdditionDialog_)
    {
        if (role == Qt::CheckStateRole && index.column () == 0)
            return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column (), role);
        else if (role == Qt::DisplayRole)
            return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
        else
            return QVariant ();
    }
    else
    {
        if (role == Qt::DisplayRole)
            return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
        else
            return QVariant ();
    }
}

Qt::ItemFlags TorrentFilesModel::flags (const QModelIndex& index) const
{
    if (!index.isValid ())
        return 0;

    if (AdditionDialog_ && index.column () == 0 && !hasChildren (index))
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    else if (!AdditionDialog_ && index.column () == 2 && !rowCount (index.sibling (index.row (), 0)))
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    else
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant TorrentFilesModel::headerData (int h, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return RootItem_->Data (h);

    return QVariant ();
}

QModelIndex TorrentFilesModel::index (int row, int col, const QModelIndex& parent) const
{
    if (!hasIndex (row, col, parent))
        return QModelIndex ();

    TreeItem *parentItem;

    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    TreeItem *childItem = parentItem->Child (row);
    if (childItem)
        return createIndex (row, col, childItem);
    else
        return QModelIndex ();
}

QModelIndex TorrentFilesModel::parent (const QModelIndex& index) const
{
    if (!index.isValid ())
        return QModelIndex ();

    TreeItem *childItem = static_cast<TreeItem*> (index.internalPointer ()),
             *parentItem = childItem->Parent ();

    if (parentItem == RootItem_)
        return QModelIndex ();

    return createIndex (parentItem->Row (), 0, parentItem);
}

int TorrentFilesModel::rowCount (const QModelIndex& parent) const
{
    TreeItem *parentItem;
    if (parent.column () > 0)
        return 0;

    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    return parentItem->ChildCount ();
}

bool TorrentFilesModel::setData (const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid ())
        return false;

    if (role == Qt::CheckStateRole)
    {
        static_cast<TreeItem*> (index.internalPointer ())->ModifyData (0, value, Qt::CheckStateRole);
        emit dataChanged (index, index);
        return true;
    }
    else if (role == Qt::EditRole && index.column () == 2)
    {
		Core::Instance ()->SetFilePriority (index.row (), value.toInt ());
        static_cast<TreeItem*> (index.internalPointer ())->ModifyData (index.column (), value);
        emit dataChanged (index, index);
        return true;
    }
    else
        return false;
}

void TorrentFilesModel::Clear ()
{
    if (!RootItem_->ChildCount ())
        return;

    beginRemoveRows (QModelIndex (), 0, RootItem_->ChildCount () - 1);
    while (RootItem_->ChildCount ())
        RootItem_->RemoveChild (0);
    endRemoveRows ();
    FilesInTorrent_ = 0;
    Path2TreeItem_.clear ();
}

void TorrentFilesModel::ResetFiles (libtorrent::torrent_info::file_iterator begin,
		const libtorrent::torrent_info::file_iterator& end)
{
    Clear ();
    beginInsertRows (QModelIndex (), 0, 0);
    int distance = std::distance (begin, end);
    if (!distance)
        return;
    FilesInTorrent_ = distance;
    Path2TreeItem_ [boost::filesystem::path ()] = RootItem_;
    for (; begin != end; ++begin)
    {
        path_t parentPath = begin->path.branch_path ();
        MkParentIfDoesntExist (begin->path);

        QList<QVariant> displayData;
        displayData << QString::fromUtf8 (begin->path.leaf ().c_str ())
			<< Proxy::Instance ()->MakePrettySize (begin->size);
        
        TreeItem *parentItem = Path2TreeItem_ [parentPath],
                 *item = new TreeItem (displayData, parentItem);
        item->ModifyData (1, static_cast<qulonglong> (begin->size), RawDataRole);
        item->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
        parentItem->AppendChild (item);
        Path2TreeItem_ [begin->path] = item;
    }
    for (int i = 0; i < RootItem_->ChildCount (); ++i)
        UpdateSizeGraph (RootItem_->Child (i));
    endInsertRows ();
}

void TorrentFilesModel::ResetFiles (const QList<FileInfo>& infos)
{
    Clear ();
    beginInsertRows (QModelIndex (), 0, 0);
    FilesInTorrent_ = infos.size ();
    Path2TreeItem_ [boost::filesystem::path ()] = RootItem_;
    for (int i = 0; i < infos.size (); ++i)
    {
        FileInfo fi = infos.at (i);
        path_t parentPath = fi.Path_.branch_path ();
        MkParentIfDoesntExist (fi.Path_);

        QList<QVariant> displayData;
        displayData << QString::fromUtf8 (fi.Path_.leaf ().c_str ())
            << Proxy::Instance ()->MakePrettySize (fi.Size_)
            << QString::number (fi.Priority_)
            << QString::number (fi.Progress_, 'f', 3);
        
        TreeItem *parentItem = Path2TreeItem_ [parentPath],
                 *item = new TreeItem (displayData, parentItem);
        item->ModifyData (1, static_cast<qulonglong> (fi.Size_), RawDataRole);
        parentItem->AppendChild (item);
        Path2TreeItem_ [fi.Path_] = item;
    }
    for (int i = 0; i < RootItem_->ChildCount (); ++i)
        UpdateSizeGraph (RootItem_->Child (i));
    endInsertRows ();
}

void TorrentFilesModel::UpdateFiles (const QList<FileInfo>& infos)
{
    if (Path2TreeItem_.isEmpty () || Path2TreeItem_.size () == 1)
    {
        ResetFiles (infos);
        return;
    }

    for (int i = 0; i < infos.size (); ++i)
    {
        FileInfo fi = infos.at (i);
		if (!Path2TreeItem_.contains (fi.Path_))
		{
			Path2TreeItem_.clear ();
			ResetFiles (infos);
		}

        TreeItem *item = Path2TreeItem_ [fi.Path_];
        item->ModifyData (3, QString::number (fi.Progress_, 'f', 3));
    }
    emit dataChanged (index (0, 3), index (RootItem_->ChildCount () - 1, 3));
}

QVector<bool> TorrentFilesModel::GetSelectedFiles () const
{
    QVector<bool> result (FilesInTorrent_);
    QList<TreeItem*> items = Path2TreeItem_.values ();
    for (int i = 0, f = 0; i < items.size (); ++i)
        if (!items.at (i)->ChildCount ())
            result [f++] = (items.at (i)->Data (0, Qt::CheckStateRole).toInt () == Qt::Checked);
    return result;
}

void TorrentFilesModel::MarkAll ()
{
    if (!RootItem_->ChildCount ())
        return;

    QList<TreeItem*> items = Path2TreeItem_.values ();
    for (int i = 0; i < items.size (); ++i)
        if (!items.at (i)->ChildCount ())
            items.at (i)->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
    emit dataChanged (index (0, 0), index (RootItem_->ChildCount () - 1, 1));
}

void TorrentFilesModel::UnmarkAll ()
{
    if (!RootItem_->ChildCount ())
        return;

    QList<TreeItem*> items = Path2TreeItem_.values ();
    for (int i = 0; i < items.size (); ++i)
        if (!items.at (i)->ChildCount ())
            items.at (i)->ModifyData (0, Qt::Unchecked, Qt::CheckStateRole);
    emit dataChanged (index (0, 0), index (RootItem_->ChildCount () - 1, 1));
}

void TorrentFilesModel::MarkIndexes (const QList<QModelIndex>& indexes)
{
    for (int i = 0; i < indexes.size (); ++i)
    {
        TreeItem *item = static_cast<TreeItem*> (indexes.at (i).internalPointer ());
        if (!item->ChildCount ())
            item->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
        emit dataChanged (index (indexes.at (i).row (), 0), index (indexes.at (i).row (), 1));
    }
}

void TorrentFilesModel::UnmarkIndexes (const QList<QModelIndex>& indexes)
{
    for (int i = 0; i < indexes.size (); ++i)
    {
        TreeItem *item = static_cast<TreeItem*> (indexes.at (i).internalPointer ());
        if (!item->ChildCount ())
            item->ModifyData (0, Qt::Unchecked, Qt::CheckStateRole);
        emit dataChanged (index (indexes.at (i).row (), 0), index (indexes.at (i).row (), 1));
    }
}

void TorrentFilesModel::MkParentIfDoesntExist (const boost::filesystem::path& path)
{
    path_t parentPath = path.branch_path ();
    if (Path2TreeItem_.contains (parentPath))
        return;

    MkParentIfDoesntExist (parentPath);
    TreeItem *parent = Path2TreeItem_ [parentPath.branch_path ()];

    QList<QVariant> data;
	data << QString::fromUtf8 (parentPath.leaf ().c_str ()) << QString ("");
	if (!AdditionDialog_)
		data << QString ("") << QString ("");
    TreeItem *item = new TreeItem (data, parent);
    parent->AppendChild (item);
    Path2TreeItem_ [parentPath] = item;
}

void TorrentFilesModel::UpdateSizeGraph (TreeItem *item)
{
    if (!item->ChildCount ())
        return;

    qulonglong size = 0;
    for (int i = 0; i < item->ChildCount (); ++i)
    {
        UpdateSizeGraph (item->Child (i));
        size += item->Child (i)->Data (1, RawDataRole).value<qulonglong> ();
    }
    item->ModifyData (1, size, RawDataRole);
    item->ModifyData (1, Proxy::Instance ()->MakePrettySize (size));
}

