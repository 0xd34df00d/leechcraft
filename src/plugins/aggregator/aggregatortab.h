/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_mainwidget.h"

namespace LC
{
namespace Util
{
	class FlatToFoldersProxyModel;
	class ShortcutManager;
}

namespace Aggregator
{
	class AppWideActions;
	class ChannelActions;
	class ChannelsFilterModel;
	class ChannelsModel;
	class DBUpdateThread;
	class ItemsWidget;
	class ResourcesFetcher;
	class UpdatesManager;

	class AggregatorTab : public QWidget
						, public ITabWidget
						, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::MainWidget Ui_;

		const TabClassInfo TabClass_;
		QObject * const ParentPlugin_;

		const std::unique_ptr<const ChannelActions> ChannelActions_;
		const std::unique_ptr<Util::FlatToFoldersProxyModel> FlatToFolders_;

		ChannelsFilterModel * const ChannelsFilterModel_;

		const std::unique_ptr<ItemsWidget> ItemsWidget_;
	public:
		struct InitParams
		{
			const TabClassInfo& TabClass_;

			const AppWideActions& AppWideActions_;
			ChannelsModel& ChannelsModel_;

			Util::ShortcutManager& ShortcutManager_;
			UpdatesManager& UpdatesManager_;
			ResourcesFetcher& ResourcesFetcher_;
			DBUpdateThread& DBUpThread_;
		};

		AggregatorTab (const InitParams&, QObject*);
		~AggregatorTab () override;

		QToolBar* GetToolBar () const override;
		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;

		QByteArray GetTabRecoverData () const override;
		QIcon GetTabRecoverIcon () const override;
		QString GetTabRecoverName () const override;
	protected:
		void keyPressEvent (QKeyEvent*) override;
	private slots:
		void handleItemsMovedToChannel (QModelIndex);
		void handleFeedsContextMenuRequested (const QPoint&);

		void currentChannelChanged ();
		void handleGroupChannels ();

		void on_MergeItems__toggled (bool);
	signals:
		void tabRecoverDataChanged () override;

		void removeTab () override;
	};
}
}
