/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QMenu>

namespace LC
{
	enum class ActionsEmbedPlace;
}

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Aggregator
{
	class ChannelsModel;
	class DBUpdateThread;
	class UpdatesManager;

	class AppWideActions : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::AppWideActions)

		QMenu ToolsMenu_;
		QList<QAction*> AllActions_;
		QList<QAction*> FastActions_;
	public:
		struct Deps
		{
			Util::ShortcutManager& ShortcutManager_;
			UpdatesManager& UpdatesManager_;
			DBUpdateThread& DBUpThread_;

			ChannelsModel& ChannelsModel_;
		};

		enum class ActionId;
		static void RegisterActions (Util::ShortcutManager&);

		explicit AppWideActions (const Deps&, QObject* = nullptr);

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
		QList<QAction*> GetFastActions () const;

		void SetEnabled (bool);
	};
}
