#include <QtDebug>
#include <plugininterface/treeitem.h>
#include <plugininterface/proxy.h>
#include <iterator>
#include "torrentfilesmodel.h"

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

void TorrentFilesModel::ResetFiles (libtorrent::torrent_info::file_iterator begin, const libtorrent::torrent_info::file_iterator& end)
{
    Clear ();
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
        if (AdditionDialog_)
            displayData << QString::fromUtf8 (begin->path.leaf ().c_str ()) << Proxy::Instance ()->MakePrettySize (begin->size);
        
        TreeItem *parentItem = Path2TreeItem_ [parentPath],
                 *item = new TreeItem (displayData, parentItem);
        item->ModifyData (1, static_cast<qulonglong> (begin->size), RawDataRole);
        if (AdditionDialog_)
            item->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
        parentItem->AppendChild (item);
    }
    for (int i = 0; i < RootItem_->ChildCount (); ++i)
        UpdateSizeGraph (RootItem_->Child (i));
    beginInsertRows (QModelIndex (), 0, RootItem_->ChildCount () - 1);
    endInsertRows ();
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

void TorrentFilesModel::MkParentIfDoesntExist (const boost::filesystem::path& path)
{
    path_t parentPath = path.branch_path ();
    if (Path2TreeItem_.contains (parentPath))
        return;

    MkParentIfDoesntExist (parentPath);
    TreeItem *parent = Path2TreeItem_ [parentPath.branch_path ()];

    QList<QVariant> data;
    data << QString::fromStdString (parentPath.leaf ()) << QString ("");
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

