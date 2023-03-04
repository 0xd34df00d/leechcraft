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

			std::function<QModelIndex ()> GetCurrentChannel_;
			std::function<QList<QModelIndex> ()> GetAllSelectedChannels_;
		};
	private:
		const Deps Deps_;
	public:
		explicit ChannelActions (const Deps&, QObject* = nullptr);

		QList<QAction*> GetAllActions () const;
		QList<QAction*> GetToolbarActions () const;
	private:
		QAction* MakeAction (const QString& name, const QByteArray& icon, auto handler, const QByteArray& actionId = {});

		void MarkAsRead (const QList<QModelIndex>&);
		void MarkAsUnread (const QList<QModelIndex>&);

		void Update (const QList<QModelIndex>&);
		void Rename (const QModelIndex&);
		void RemoveFeed (const QList<QModelIndex>&);

		void RemoveChannel (const QList<QModelIndex>&);

		void Settings (const QModelIndex&);
	};
}
