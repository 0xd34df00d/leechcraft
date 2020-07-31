/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mainwindowmenumanager.h"
#include <QMenu>
#include <util/sll/qtutil.h>
#include <util/sll/unreachable.h>
#include "interfaces/iactionsexporter.h"
#include "ui_leechcraft.h"
#include "core.h"
#include "newtabmenumanager.h"
#include "iconthemeengine.h"

namespace LC
{
	MainWindowMenuManager::MainWindowMenuManager (const Ui::LeechCraft& ui, QObject *parent)
	: QObject { parent }
	, Menu_ { new QMenu }
	{
		MenuView_ = new QMenu (tr ("View"), Menu_.get ());
		MenuView_->addSeparator ();
		MenuView_->addAction (ui.ActionShowStatusBar_);
		MenuView_->addAction (ui.ActionFullscreenMode_);
		MenuTools_ = new QMenu (tr ("Tools"), Menu_.get ());

		Menu_->addAction (ui.ActionNewWindow_);
		Menu_->addMenu (Core::Instance ().GetNewTabMenuManager ()->GetNewTabMenu ());
		Menu_->addSeparator ();
		Menu_->addAction (ui.ActionAddTask_);
		Menu_->addSeparator ();
		Menu_->addMenu (MenuTools_);
		Menu_->addMenu (MenuView_);
		Menu_->addSeparator ();
		Menu_->addAction (ui.ActionSettings_);
		Menu_->addSeparator ();
		Menu_->addAction (ui.ActionAboutLeechCraft_);
		Menu_->addSeparator ();
		Menu_->addAction (ui.ActionRestart_);
		Menu_->addAction (ui.ActionQuit_);
	}

	QMenu* MainWindowMenuManager::GetMenu () const
	{
		return Menu_.get ();
	}

	QMenu* MainWindowMenuManager::GetSubMenu (Role role) const
	{
		switch (role)
		{
		case Role::Tools:
			return MenuTools_;
		case Role::View:
			return MenuView_;
		}

		Util::Unreachable ();
	}

	void MainWindowMenuManager::FillToolMenu()
	{
		for (const auto exporter : Core::Instance ().GetPluginManager ()->
					GetAllCastableTo<IActionsExporter*> ())
		{
			const auto& acts = exporter->GetActions (ActionsEmbedPlace::ToolsMenu);
			IconThemeEngine::Instance ().UpdateIconset (acts);
			if (acts.size ())
				MenuTools_->addSeparator ();
			MenuTools_->addActions (acts);
		}
	}

	void MainWindowMenuManager::AddMenus (const QMap<QString, QList<QAction*>>& menus)
	{
		for (const auto& pair : Util::Stlize (menus))
		{
			const auto& menuName = pair.first;
			QMenu *toInsert = nullptr;
			if (menuName == "view")
				toInsert = MenuView_;
			else if (menuName == "tools")
				toInsert = MenuTools_;
			else
				for (auto action : Menu_->actions ())
					if (action->menu () &&
						action->text () == menuName)
					{
						toInsert = action->menu ();
						break;
					}

			const auto& actions = pair.second;

			if (toInsert)
				toInsert->insertActions (toInsert->actions ().value (0, 0), actions);
			else
			{
				auto menu = new QMenu { menuName, Menu_.get () };
				menu->addActions (actions);
				Menu_->insertMenu (MenuTools_->menuAction (), menu);
			}

			IconThemeEngine::Instance ().UpdateIconset (actions);
		}
	}

	void MainWindowMenuManager::RemoveMenus (const QMap<QString, QList<QAction*>>& menus)
	{
		for (const auto& pair : Util::Stlize (menus))
		{
			const auto& menuName = pair.first;

			QMenu *toRemove = nullptr;
			if (menuName == "view")
				toRemove = MenuView_;
			else if (menuName == "tools")
				toRemove = MenuTools_;

			const auto& actions = pair.second;

			if (toRemove)
				for (const auto action : actions)
					toRemove->removeAction (action);
			else
			{
				for (auto action : Menu_->actions ())
					if (action->text () == menuName)
					{
						Menu_->removeAction (action);
						break;
					}
			}
		}
	}
}
