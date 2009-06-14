#ifndef PLUGINS_BITTORRENT_TORRENTFILESMODEL_H
#define PLUGINS_BITTORRENT_TORRENTFILESMODEL_H
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

	namespace Plugins
	{
		namespace BitTorrent
		{
			struct Hash : public std::unary_function<boost::filesystem::path, size_t>
			{
				boost::hash<std::string> H_;

				size_t operator() (const boost::filesystem::path& p) const
				{
					return H_ (p.string ());
				}
			};

			struct MyEqual : public std::binary_function<boost::filesystem::path,
				boost::filesystem::path,
				bool>
			{
				bool operator() (const boost::filesystem::path& p1,
						const boost::filesystem::path& p2) const
				{
					return p1.string () == p2.string ();
				}
			};

			typedef boost::unordered_map<boost::filesystem::path,
					LeechCraft::Util::TreeItem*,
					Hash,
					MyEqual> Path2TreeItem_t;

			class TorrentFilesModel : public QAbstractItemModel
			{
				Q_OBJECT

				LeechCraft::Util::TreeItem *RootItem_;
				bool AdditionDialog_;
				Path2TreeItem_t Path2TreeItem_;
				int FilesInTorrent_;
				enum
				{
					RawDataRole = 46,
					RolePath
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
		};
	};
};

#endif

