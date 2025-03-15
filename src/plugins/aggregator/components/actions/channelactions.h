/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QCoreApplication>
#include <QMenu>
#include <QList>

class QModelIndex;

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Aggregator
{
	class DBUpdateThread;
	class ResourcesFetcher;
	class UpdatesManager;
	struct ChannelShort;

	class ChannelActions : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ItemActions)

		QList<QAction*> AllActions_;
		QList<QAction*> ToolbarActions_;
	public:
		struct Deps
		{
			Util::ShortcutManager& ShortcutManager_;
			UpdatesManager& UpdatesManager_;
			ResourcesFetcher& ResourcesFetcher_;
			DBUpdateThread& DBUpThread_;

			std::function<std::optional<ChannelShort> ()> GetCurrentChannel_;
			std::function<QList<ChannelShort> ()> GetAllSelectedChannels_;
		};
	private:
		const Deps Deps_;
	public:
		enum class ActionId;
		static void RegisterActions (Util::ShortcutManager&);

		explicit ChannelActions (const Deps&, QObject* = nullptr);

		QList<QAction*> GetAllActions () const;
		QList<QAction*> GetToolbarActions () const;
	private:
		QAction* MakeAction (ActionId, auto handler);

		void MarkAsRead (const QList<ChannelShort>&);
		void MarkAsUnread (const QList<ChannelShort>&);

		void Update (const QList<ChannelShort>&);
		void Rename (const ChannelShort&);
		void RemoveFeed (const QList<ChannelShort>&);

		void RemoveChannel (const QList<ChannelShort>&);

		void Settings (const ChannelShort&);
	};
}
