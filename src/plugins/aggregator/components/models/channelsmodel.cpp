/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelsmodel.h"
#include <stdexcept>
#include <algorithm>
#include <ranges>
#include <QtDebug>
#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QIcon>
#include <QFontMetrics>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/xpc/downloaderrorstrings.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/itagsmanager.h>
#include "components/storage/storagebackendmanager.h"
#include "item.h"
#include "xmlsettingsmanager.h"
#include "feedserrormanager.h"
#include "tooltipbuilder.h"

namespace LC::Aggregator
{
	ChannelsModel::ChannelsModel (const std::shared_ptr<const FeedsErrorManager>& errorMgr,
			const ITagsManager *itm, QObject *parent)
	: QAbstractItemModel { parent }
	, Headers_ { tr ("Feed"), tr ("Unread items"), tr ("Last build") }
	, TagsManager_ { itm }
	, FeedsErrorManager_ { errorMgr }
	{
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::channelRemoved,
				this,
				&ChannelsModel::RemoveChannel);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::feedRemoved,
				this,
				&ChannelsModel::RemoveFeed);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::channelAdded,
				this,
				[this] (const Channel& channel) { AddChannel (channel.ToShort ()); });
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::channelUnreadCountUpdated,
				this,
				&ChannelsModel::UpdateChannelUnreadCount,
				Qt::QueuedConnection);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::channelDataUpdated,
				this,
				&ChannelsModel::UpdateChannelData,
				Qt::QueuedConnection);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::storageCreated,
				this,
				&ChannelsModel::PopulateChannels);

		connect (FeedsErrorManager_.get (),
				&FeedsErrorManager::gotErrors,
				this,
				&ChannelsModel::HandleFeedErrorsChanged);
		connect (FeedsErrorManager_.get (),
				&FeedsErrorManager::clearedErrors,
				this,
				&ChannelsModel::HandleFeedErrorsChanged);

		if (StorageBackendManager::Instance ().IsPrimaryStorageCreated ())
			PopulateChannels ();
	}

	int ChannelsModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	namespace
	{
		QVariant GetForegroundColor (const ChannelShort& cs)
		{
			bool palette = XmlSettingsManager::Instance ().property ("UsePaletteColors").toBool ();
			if (cs.Unread_)
			{
				if (XmlSettingsManager::Instance ().property ("UnreadCustomColor").toBool ())
					return XmlSettingsManager::Instance ().property ("UnreadItemsColor");
				else
					return palette ?
							QApplication::palette ().link ().color () :
							QVariant {};
			}
			else
				return palette ?
						QApplication::palette ().linkVisited ().color () :
						QVariant {};
		}

		QString ErrorToString (const FeedsErrorManager::Error& error)
		{
			return Util::Visit (error,
					[] (const FeedsErrorManager::ParseError& e) { return ChannelsModel::tr ("Parse error: ") + e.Message_; },
					[] (const IDownload::Error& e)
					{
						auto str = ChannelsModel::tr ("Error downloading the feed: %1.")
								.arg (Util::GetErrorString (e.Type_));
						if (!e.Message_.isEmpty ())
							str += " " + e.Message_;
						return str;
					});
		}

		QVariant GetTooltip (const ITagsManager *itm, const FeedsErrorManager& errMgr, const ChannelShort& cs)
		{
			const auto& errors = errMgr.GetFeedErrors (cs.FeedID_);
			if (!errors.isEmpty ())
			{
				auto errorsStrings = Util::Map (errors, ErrorToString);
				errorsStrings.removeDuplicates ();
				return errorsStrings.join ('\n');
			}

			auto tb = TooltipBuilder { cs.Title_ }
					.Add (ChannelsModel::tr ("Author"), cs.Author_)
					.Add (ChannelsModel::tr ("Tags"), itm->JoinIDs (cs.Tags_));

			auto elidedLink = QFontMetrics { QApplication::font () }.elidedText (cs.Link_, Qt::ElideMiddle, 400);
			tb += u"<a href='%1'>%2</a>"_qsv
					.arg (cs.Link_,
						  elidedLink);

			return tb.GetTooltip ();
		}
	}

	namespace
	{
		QString GetChannelTitle (const ChannelShort& channel)
		{
			return channel.DisplayTitle_.isEmpty () ?
					channel.Title_ :
					channel.DisplayTitle_;
		}
	}

	QVariant ChannelsModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto row = index.row ();
		const auto column = index.column ();

		const auto& channel = Channels_.at (row);
		switch (role)
		{
		case Qt::DisplayRole:
			switch (column)
			{
			case ColumnTitle:
				return GetChannelTitle (channel);
			case ColumnUnread:
				return channel.Unread_;
			case ColumnLastBuild:
				return channel.LastBuild_;
			default:
				return {};
			}
		case Qt::DecorationRole:
			if (column == ColumnTitle)
			{
				QIcon result = QPixmap::fromImage (channel.Favicon_);
				if (result.isNull ())
					result = GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("application-rss+xml"_qs);
				if (result.isNull ())
					result = GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
				return result;
			}
			return {};
		case Qt::ForegroundRole:
			return GetForegroundColor (channel);
		case Qt::FontRole:
			if (const auto& errors = FeedsErrorManager_->GetFeedErrors (channel.FeedID_); !errors.isEmpty ())
			{
				QFont font;
				font.setStrikeOut (true);
				return font;
			}
			if (channel.Unread_)
				return XmlSettingsManager::Instance ().property ("UnreadItemsFont");
			return {};
		case Qt::ToolTipRole:
			return GetTooltip (TagsManager_, *FeedsErrorManager_, channel);
		case RoleTags:
			return channel.Tags_;
		case ChannelRoles::UnreadCount:
			return channel.Unread_;
		case ChannelRoles::ErrorCount:
			return FeedsErrorManager_->GetFeedErrors (channel.FeedID_).size ();
		case ChannelRoles::ChannelID:
			return channel.ChannelID_;
		case ChannelRoles::FeedID:
			return channel.FeedID_;
		case ChannelRoles::RawTags:
			return channel.Tags_;
		case ChannelRoles::HumanReadableTags:
			return TagsManager_->GetTags (channel.Tags_);
		case ChannelRoles::ChannelTitle:
			return GetChannelTitle (channel);
		case ChannelRoles::ChannelLink:
			return channel.Link_;
		case ChannelRoles::ChannelShortStruct:
			return QVariant::fromValue (channel);
		default:
			return {};
		}
	}

	Qt::ItemFlags ChannelsModel::flags (const QModelIndex&) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant ChannelsModel::headerData (int column, Qt::Orientation orient, int role) const
	{
		if (orient == Qt::Horizontal && role == Qt::DisplayRole)
			return Headers_.at (column);
		else
			return {};
	}

	QModelIndex ChannelsModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		return createIndex (row, column);
	}

	QModelIndex ChannelsModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int ChannelsModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Channels_.size ();
	}

	void ChannelsModel::AddChannel (const ChannelShort& channel)
	{
		beginInsertRows ({}, rowCount (), rowCount ());
		Channels_ << channel;
		endInsertRows ();
	}

	const ChannelShort& ChannelsModel::GetChannelForIndex (const QModelIndex& index) const
	{
		if (!index.isValid ())
			throw std::runtime_error ("Invalid index");

		return Channels_ [index.row ()];
	}

	QModelIndexList ChannelsModel::FindItems (const QSet<IDType_t>& ids) const
	{
		QModelIndexList result;
		result.reserve (ids.size ());
		for (const auto& [channel, row] : std::views::zip (Channels_, std::views::iota (0)))
			if (ids.contains (channel.ChannelID_))
				result << index (row, 0);
		return result;
	}

	void ChannelsModel::Clear ()
	{
		beginResetModel ();
		Channels_.clear ();
		endResetModel ();
	}

	namespace
	{
		auto FindById (QList<ChannelShort>& channels, IDType_t id)
		{
			return std::ranges::find_if (channels, [id] (const ChannelShort& cs) { return cs.ChannelID_ == id; });
		}
	}

	void ChannelsModel::RemoveChannel (IDType_t id)
	{
		const auto pos = FindById (Channels_, id);
		if (pos == Channels_.end ())
			return;

		const auto idx = pos - Channels_.begin ();
		beginRemoveRows ({}, idx, idx);
		Channels_.erase (pos);
		endRemoveRows ();
	}

	void ChannelsModel::RemoveFeed (IDType_t id)
	{
		for (auto it = Channels_.begin (); it != Channels_.end ();)
		{
			if (it->FeedID_ != id)
			{
				++it;
				continue;
			}

			const auto idx = it - Channels_.begin ();
			beginRemoveRows ({}, idx, idx);
			it = Channels_.erase (it);
			endRemoveRows ();
		}
	}

	void ChannelsModel::HandleFeedErrorsChanged (IDType_t feedId)
	{
		for (auto i = 0; i < Channels_.size (); ++i)
			if (Channels_.at (i).FeedID_ == feedId)
				emit dataChanged (index (i, 0), index (i, columnCount () - 1));
	}

	void ChannelsModel::UpdateChannelUnreadCount (IDType_t cid, const UnreadChange& count)
	{
		const auto pos = FindById (Channels_, cid);
		if (pos == Channels_.end ())
			return;

		Util::Visit (count,
				[pos] (UnreadDelta d) { pos->Unread_ += d.delta; },
				[pos] (UnreadTotal t) { pos->Unread_ = t.total; });

		const auto idx = pos - Channels_.begin ();
		emit dataChanged (index (idx, 0), index (idx, 2));
	}

	void ChannelsModel::UpdateChannelData (const Channel& channel)
	{
		const auto pos = FindById (Channels_, channel.ChannelID_);
		if (pos == Channels_.end ())
			return;

		const auto unreadCount = pos->Unread_;
		*pos = channel.ToShort ();
		pos->Unread_ = unreadCount;

		const auto idx = pos - Channels_.begin ();
		emit dataChanged (index (idx, 0), index (idx, 2));
	}

	void ChannelsModel::PopulateChannels ()
	{
		Clear ();

		auto storage = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto feedId : storage->GetFeedsIDs ())
			for (const auto& chan : storage->GetChannels (feedId))
				AddChannel (chan);
	}
}
