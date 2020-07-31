/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>

class QMenu;
class QAction;

namespace Ui
{
	class LeechCraft;
}

namespace LC
{
	class MainWindowMenuManager : public QObject
	{
		Q_OBJECT

		std::shared_ptr<QMenu> const Menu_;

		QMenu *MenuView_;
		QMenu *MenuTools_;
	public:
		enum class Role
		{
			Tools,
			View
		};

		MainWindowMenuManager (const Ui::LeechCraft&, QObject* = nullptr);

		QMenu* GetMenu () const;

		QMenu* GetSubMenu (Role) const;

		void FillToolMenu ();

		void AddMenus (const QMap<QString, QList<QAction*>>&);
		void RemoveMenus (const QMap<QString, QList<QAction*>>&);
	};
}
