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

#ifndef PLUGINS_AGGREGATOR_CHANNELSMODEL_H
#define PLUGINS_AGGREGATOR_CHANNELSMODEL_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include "channel.h"

class QToolBar;
class QMenu;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ChannelsModel : public QAbstractItemModel
			{
				Q_OBJECT

				QStringList Headers_;
				typedef QList<ChannelShort> Channels_t;
				Channels_t Channels_;
				QToolBar *Toolbar_;
				QWidget *TabWidget_;
				QMenu *Menu_;
			public:
				enum Columns
				{
					ColumnTitle,
					ColumnUnread,
					ColumnLastBuild
				};
				ChannelsModel (QObject *parent = 0);
				virtual ~ChannelsModel ();

				void SetWidgets (QToolBar*, QWidget*);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				void AddChannel (const ChannelShort&);
				void Update (const channels_container_t&);
				void UpdateChannelData (const ChannelShort&);
				ChannelShort& GetChannelForIndex (const QModelIndex&);
				void RemoveChannel (const ChannelShort&);
				QModelIndex GetUnreadChannelIndex () const;
				int GetUnreadChannelsNumber () const;
				int GetUnreadItemsNumber () const;
				void SetMenu (QMenu*);
			signals:
				void channelDataUpdated ();
			};
		};
	};
};

#endif

