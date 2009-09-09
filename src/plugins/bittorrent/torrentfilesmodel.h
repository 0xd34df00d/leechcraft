/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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

			typedef boost::unordered_map<boost::filesystem::path,
					int,
					Hash,
					MyEqual> Path2Position_t;

			class TorrentFilesModel : public QAbstractItemModel
			{
				Q_OBJECT

				LeechCraft::Util::TreeItem *RootItem_;
				bool AdditionDialog_;
				Path2TreeItem_t Path2TreeItem_;
				Path2Position_t Path2OriginalPosition_;
				int FilesInTorrent_;
			public:
				enum
				{
					RawDataRole = 46,
					RolePath,
					RoleSize,
					RoleProgress
				};
                enum
                {
                    ColumnPath,
                    ColumnPriority,
                    ColumnProgress
                };

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

