#ifndef TORRENTFILESMODEL_H
#define TORRENTFILESMODEL_H
#include <QAbstractItemModel>
#include <libtorrent/torrent_info.hpp>

class TreeItem;

class TorrentFilesModel : public QAbstractItemModel
{
    Q_OBJECT

    TreeItem *RootItem_;
    bool AdditionDialog_;
    typedef boost::filesystem::path path_t;
    QMap<path_t, TreeItem*> Path2TreeItem_;
    int FilesInTorrent_;
    enum { RawDataRole = 46 } ;
public:
    TorrentFilesModel (bool, QObject *parent = 0);
    virtual ~TorrentFilesModel ();

    virtual int columnCount (const QModelIndex&) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    void Clear ();
    void ResetFiles (libtorrent::torrent_info::file_iterator, const libtorrent::torrent_info::file_iterator&);
    QVector<bool> GetSelectedFiles () const;
private:
    void MkParentIfDoesntExist (const boost::filesystem::path&);
    void UpdateSizeGraph (TreeItem*);
};

#endif

