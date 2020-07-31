/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pierre.h"
#include <QIcon>
#include <QMenuBar>
#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>
#include <interfaces/iactionsexporter.h>
#include "fullscreen.h"
#include "reopenhandler.h"

namespace LC
{
namespace Pierre
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		TrayIconMenu_ = 0;

		Proxy_ = proxy;
		MenuBar_ = new QMenuBar (0);

		ReopenHandler::Instance ().SetCoreProxy (proxy);
	}

	void Plugin::SecondInit ()
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetQObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Pierre";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Pierre";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Pierre d'Olle is the Mac OS X integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void Plugin::hookGonnaFillMenu (IHookProxy_ptr)
	{
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::hookTrayIconCreated (IHookProxy_ptr proxy, QSystemTrayIcon *icon)
	{
		TrayIconMenu_ = icon->contextMenu ();
		TrayIconMenu_->setAsDockMenu ();
	}

	void Plugin::hookTrayIconVisibilityChanged (IHookProxy_ptr proxy, QSystemTrayIcon*, bool)
	{
		proxy->CancelDefault ();
	}

	void Plugin::handleGotActions (const QList<QAction*>&, ActionsEmbedPlace aep)
	{
		if (aep != ActionsEmbedPlace::ToolsMenu)
			return;

		MenuBar_->clear ();
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::handleWindow (int index)
	{
		qDebug () << Q_FUNC_INFO;
		auto rootWM = Proxy_->GetRootWindowsManager ();
		FS::AddAction (rootWM->GetMainWindow (index));
	}

	void Plugin::fillMenu ()
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		auto menu = rootWM->GetMWProxy (0)->GetMainMenu ();

		QMenu *lcMenu = 0;
		QList<QAction*> firstLevelActions;
		for (auto action : menu->actions ())
			if (action->menu ())
			{
				MenuBar_->addAction (action);
				if (!lcMenu)
					lcMenu = action->menu ();
			}
			else
			{
				if (action->menuRole () == QAction::TextHeuristicRole)
					action->setMenuRole (QAction::ApplicationSpecificRole);
				firstLevelActions << action;
			}

		for (auto act : firstLevelActions)
			lcMenu->addAction (act);

		if (!lcMenu->actions ().isEmpty ())
			MenuBar_->addMenu (lcMenu);

		const auto& actors = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		for (auto actor : actors)
			connect (actor,
					SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
					Qt::UniqueConnection);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pierre, LC::Pierre::Plugin);
