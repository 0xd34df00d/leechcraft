/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QList>
#include <QObject>
#include "../../common.h"
#include "../gui/itemnavigator.h"

class QAction;

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Aggregator
{
	class UpdatesManager;

	class ItemActions : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ItemActions)

		QList<QAction*> AllActions_;
		QList<QAction*> ToolbarActions_;
		QList<QAction*> InvisibleActions_;

		QAction *MarkUnimportant_ = nullptr;
		QAction *MarkImportant_ = nullptr;
		QAction *MarkUnread_ = nullptr;
		QAction *MarkRead_ = nullptr;
		QAction *Delete_ = nullptr;

		QAction *LinkOpen_ = nullptr;
		QAction *LinkCopy_ = nullptr;
		QAction *SubComments_ = nullptr;
	public:
		struct Deps
		{
			QWidget * const Parent_;

			Util::ShortcutManager& ShortcutsMgr_;
			UpdatesManager& UpdatesManager_;

			std::function<void (bool)> SetHideRead_;
			std::function<void (bool)> SetShowTape_;

			std::function<QList<QModelIndex> ()> GetSelection_;

			ItemNavigator ItemNavigator_;
		};
	private:
		const Deps Deps_;
	public:
		explicit ItemActions (const Deps&, QObject*);

		QList<QAction*> GetAllActions () const;
		QList<QAction*> GetToolbarActions () const;
		QList<QAction*> GetInvisibleActions () const;

		void HandleSelectionChanged (const QList<QModelIndex>&);
	private:
		struct ActionInfo;

		QAction* MakeAction (const QString& name,
				const QByteArray& icon,
				const QByteArray& objectNameSuffix,
				auto handler,
				const ActionInfo& info);

		void MarkSelectedReadStatus (bool read);
		void MarkSelectedAsImportant (bool important);

		void DeleteSelected ();

		void SubscribeComments ();
		void LinkOpen ();
		void LinkCopy ();

		QVector<IDType_t> GetSelectedIds () const;
	};
}
