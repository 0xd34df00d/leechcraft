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

#include <stdexcept>
#include <QtDebug>
#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QIcon>
#include <interfaces/structures.h>
#include "channelsmodel.h"
#include "item.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ChannelsModel::ChannelsModel (QObject *parent)
			: QAbstractItemModel (parent)
			, Toolbar_ (0)
			, TabWidget_ (0)
			, Menu_ (0)
			{
				setObjectName ("Aggregator ChannelsModel");
				Headers_ << tr ("Feed")
					<< tr ("Unread items")
					<< tr ("Last build");
			}
			
			ChannelsModel::~ChannelsModel ()
			{
			}
			
			void ChannelsModel::SetWidgets (QToolBar *bar, QWidget *tab)
			{
				Toolbar_ = bar;
				TabWidget_ = tab;
			}
			
			int ChannelsModel::columnCount (const QModelIndex&) const
			{
				return Headers_.size ();
			}
			
			QVariant ChannelsModel::data (const QModelIndex& index, int role) const
			{
				if (role == LeechCraft::RoleControls)
					return QVariant::fromValue<QToolBar*> (Toolbar_);
				if (role == LeechCraft::RoleAdditionalInfo)
					return QVariant::fromValue<QWidget*> (TabWidget_);
				if (role == LeechCraft::RoleContextMenu)
					return QVariant::fromValue<QMenu*> (Menu_);
			
				if (!index.isValid ())
					return QVariant ();
			
				int row = index.row ();
				if (role == Qt::DisplayRole)
					switch (index.column ())
					{
						case ColumnTitle:
							return Channels_.at (row).Title_;
						case ColumnUnread:
							return Channels_.at (row).Unread_;
						case ColumnLastBuild:
							return Channels_.at (row).LastBuild_;
						default:
							return QVariant ();
					}
				else if (role == Qt::DecorationRole &&
						index.column () == 0)
				{
					QIcon result = Channels_.at (row).Favicon_;
					if (result.isNull ())
						result = QIcon (":/resources/images/rss.png");
					return result;
				}
				else if (role == Qt::ForegroundRole)
					if (Channels_.at (row).Unread_ &&
							XmlSettingsManager::Instance ()->
							property ("UnreadCustomColor").toBool ())
						return XmlSettingsManager::Instance ()->
							property ("UnreadItemsColor").value<QColor> ();
					else
						return QVariant ();
				else if (role == Qt::FontRole)
				{
					if (Channels_.at (row).Unread_)
						return XmlSettingsManager::Instance ()->
							property ("UnreadItemsFont");
					else
						return QVariant ();
				}
				else if (role == LeechCraft::RoleTags)
					return Channels_.at (row).Tags_;
				else
					return QVariant ();
			}
			
			Qt::ItemFlags ChannelsModel::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;
				else
					return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}
			
			QVariant ChannelsModel::headerData (int column, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return Headers_.at (column);
				else
					return QVariant ();
			}
			
			QModelIndex ChannelsModel::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex ChannelsModel::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int ChannelsModel::rowCount (const QModelIndex& parent) const
			{
				return parent.isValid () ? 0 : Channels_.size ();
			}
			
			void ChannelsModel::AddChannel (const ChannelShort& channel)
			{
				beginInsertRows (QModelIndex (), rowCount (), rowCount ());
				Channels_ << channel;
				endInsertRows ();
			}
			
			void ChannelsModel::Update (const channels_container_t& channels)
			{
				for (size_t i = 0; i < channels.size (); ++i)
				{
					Channels_t::const_iterator pos =
						std::find (Channels_.begin (), Channels_.end (),
							channels.at (i));
					if (pos != Channels_.end ())
						continue;
			
					Channels_ << channels [i]->ToShort ();
				}
			}
			
			void ChannelsModel::UpdateChannelData (const ChannelShort& cs)
			{
				Channels_t::iterator idx =
					std::find (Channels_.begin (), Channels_.end (), cs);
				if (idx == Channels_.end ())
					return;
				*idx = cs;
				int pos = std::distance (Channels_.begin (), idx);
				emit dataChanged (index (pos, 0), index (pos, 2));
				emit channelDataUpdated ();
			}
			
			ChannelShort& ChannelsModel::GetChannelForIndex (const QModelIndex& index)
			{
				if (!index.isValid ())
					throw std::runtime_error ("Invalid index");
				else
					return Channels_ [index.row ()];
			}
			
			void ChannelsModel::RemoveChannel (const ChannelShort& channel)
			{
				Channels_t::iterator idx =
					std::find (Channels_.begin (), Channels_.end (), channel);
				if (idx == Channels_.end ())
					return;
			
				Channels_.erase (idx);
				reset ();
			}
			
			QModelIndex ChannelsModel::GetUnreadChannelIndex () const
			{
				for (int i = 0; i < Channels_.size (); ++i)
					if (Channels_.at (i).Unread_)
						return index (i, 0);
				return QModelIndex ();
			}
			
			int ChannelsModel::GetUnreadChannelsNumber () const
			{
				int result = 0;
				for (int i = 0; i < Channels_.size (); ++i)
					if (Channels_.at (i).Unread_)
						++result;
				return result;
			}
			
			int ChannelsModel::GetUnreadItemsNumber () const
			{
				int result = 0;
				for (int i = 0; i < Channels_.size (); ++i)
					result += Channels_.at (i).Unread_;
				return result;
			}

			void ChannelsModel::SetMenu (QMenu *menu)
			{
				Menu_ = menu;
			}
		};
	};
};

