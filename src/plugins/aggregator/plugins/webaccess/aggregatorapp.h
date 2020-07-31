/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <Wt/WApplication.h>
#include <Wt/WModelIndex.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/aggregator/item.h>

class QThread;

namespace LC
{
namespace Aggregator
{
class IProxyObject;

namespace WebAccess
{
	class Q2WProxyModel;
	class ReadChannelsFilter;
	class ReadItemsFilter;

	class AggregatorApp : public Wt::WApplication
	{
		Q_DECLARE_TR_FUNCTIONS (AggregatorApp)

		IProxyObject *AP_;
		ICoreProxy_ptr CP_;

		QThread * const ObjsThread_;

		std::shared_ptr<Q2WProxyModel> ChannelsModel_;
		std::shared_ptr<ReadChannelsFilter> ChannelsFilter_;

		QAbstractItemModel * const SourceItemModel_;
		std::shared_ptr<Q2WProxyModel> ItemsModel_;
		std::shared_ptr<ReadItemsFilter> ItemsFilter_;

		Wt::WTableView *ItemsTable_;

		Wt::WText *ItemView_;
	public:
		enum ChannelRole
		{
			CID = Wt::ItemDataRole::User + 1,
			FID,
			UnreadCount
		};

		enum ItemRole
		{
			IID = Wt::ItemDataRole::User + 1,
			IsRead
		};

		AggregatorApp (IProxyObject*, ICoreProxy_ptr, const Wt::WEnvironment& environment);
		~AggregatorApp ();
	private:
		void HandleChannelClicked (const Wt::WModelIndex&, const Wt::WMouseEvent&);
		void HandleItemClicked (const Wt::WModelIndex&, const Wt::WMouseEvent&);

		void ShowItem (const QModelIndex&, const Item&);
		void ShowItemMenu (const QModelIndex&, const Item&, const Wt::WMouseEvent&);

		void SetupUI ();
	};
}
}
}
