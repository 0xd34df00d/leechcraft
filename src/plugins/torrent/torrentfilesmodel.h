#ifndef TORRENTFILESMODEL_H
#define TORRENTFILESMODEL_H
#include <QAbstractItemModel>
#include <boost/unordered_map.hpp>
#include <libtorrent/torrent_info.hpp>
#include "fileinfo.h"

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem;
	};
};

typedef boost::unordered_map<boost::filesystem::path, LeechCraft::Util::TreeItem*> Path2TreeItem_t;

namespace boost
{
	template<>
	struct hash<boost::filesystem::path> : public std::unary_function<boost::filesystem::path, size_t>
	{
		hash<std::string> h;

		size_t operator() (boost::filesystem::path const& p) const
		{
			return h (p.string ());
		}
	};
};

class TorrentFilesModel : public QAbstractItemModel
{
    Q_OBJECT

    LeechCraft::Util::TreeItem *RootItem_;
    bool AdditionDialog_;
	Path2TreeItem_t Path2TreeItem_;
    int FilesInTorrent_;
    enum
	{
		RawDataRole = 46
	};
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
    void ResetFiles (libtorrent::torrent_info::file_iterator,
			const libtorrent::torrent_info::file_iterator&);
    void ResetFiles (const QList<FileInfo>&);
    void UpdateFiles (const QList<FileInfo>&);
    QVector<bool> GetSelectedFiles () const;
    void MarkAll ();
    void UnmarkAll ();
    void MarkIndexes (const QList<QModelIndex>&);
    void UnmarkIndexes (const QList<QModelIndex>&);
private:
    void MkParentIfDoesntExist (const boost::filesystem::path&);
    void UpdateSizeGraph (LeechCraft::Util::TreeItem*);
};

#endif

