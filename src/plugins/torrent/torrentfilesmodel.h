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
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
    virtual bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole);

    void Clear ();
    void ResetFiles (libtorrent::torrent_info::file_iterator, const libtorrent::torrent_info::file_iterator&);
    QVector<bool> GetSelectedFiles () const;
    void MarkAll ();
    void UnmarkAll ();
    void MarkIndexes (const QList<QModelIndex>&);
    void UnmarkIndexes (const QList<QModelIndex>&);
private:
    void MkParentIfDoesntExist (const boost::filesystem::path&);
    void UpdateSizeGraph (TreeItem*);
};

#endif

