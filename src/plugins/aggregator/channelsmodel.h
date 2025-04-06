/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QAbstractItemModel>
#include "channel.h"

class QToolBar;
class QMenu;

class ITagsManager;

namespace LC::Aggregator
{
	class FeedsErrorManager;
	struct UnreadChange;

	class ChannelsModel : public QAbstractItemModel
	{
		Q_DECLARE_TR_FUNCTIONS (ChannelsModel)

		const QStringList Headers_;
		QList<ChannelShort> Channels_;

		const ITagsManager * const TagsManager_;
		const std::shared_ptr<const FeedsErrorManager> FeedsErrorManager_;
	public:
		enum Columns
		{
			ColumnTitle,
			ColumnUnread,
			ColumnLastBuild
		};
		explicit ChannelsModel (const std::shared_ptr<const FeedsErrorManager>&,
		        const ITagsManager*, QObject *parent = nullptr);

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;

		const ChannelShort& GetChannelForIndex (const QModelIndex&) const;
		QModelIndexList FindItems (const QSet<IDType_t>&) const;
	private:
		void RemoveChannel (IDType_t);
		void RemoveFeed (IDType_t);

		void HandleFeedErrorsChanged (IDType_t);

		void UpdateChannelUnreadCount (IDType_t, const UnreadChange&);
		void UpdateChannelData (const Channel&);

		void AddChannel (const ChannelShort&);
		void Clear ();
		void PopulateChannels ();
	};
}
