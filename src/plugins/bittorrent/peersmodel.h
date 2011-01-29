/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_BITTORRENT_PEERSMODEL_H
#define PLUGINS_BITTORRENT_PEERSMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include "peerinfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class PeersModel : public QAbstractItemModel
			{
				Q_OBJECT

				friend class Core;

				QStringList Headers_;
				QList<PeerInfo> Peers_;
				int CurrentTorrent_;
			public:
				enum { SortRole = 45 };
				PeersModel (QObject *parent = 0);
				virtual ~PeersModel ();

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

				const PeerInfo& GetPeerInfo (const QModelIndex&) const;
			protected:
				void Clear ();
				void Update (const QList<PeerInfo>&, int);
			};
		};
	};
};

#endif

