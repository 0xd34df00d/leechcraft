/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "pierre.h"
#include <QIcon>
#include <QMenuBar>
#include <QMainWindow>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/imwproxy.h>
#include <interfaces/iactionsexporter.h>
#include "fullscreen.h"

namespace LeechCraft
{
namespace Pierre
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		MenuBar_ = new QMenuBar (0);

		FS::AddAction (Proxy_->GetMainWindow ());
	}

	void Plugin::SecondInit ()
	{
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

	void Plugin::handleGotActions (const QList<QAction*>&, ActionsEmbedPlace aep)
	{
		if (aep != ActionsEmbedPlace::ToolsMenu)
			return;

		MenuBar_->clear ();
		QTimer::singleShot (0,
				this,
				SLOT (fillMenu ()));
	}

	void Plugin::fillMenu ()
	{
		auto menu = Proxy_->GetMWProxy ()->GetMainMenu ();

		QMenu *lcMenu = 0;
		QList<QAction*> firstLevelActions;
		Q_FOREACH (auto action, menu->actions ())
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

		Q_FOREACH (auto act, firstLevelActions)
			lcMenu->addAction (act);

		if (!lcMenu->actions ().isEmpty ())
			MenuBar_->addMenu (lcMenu);

		const auto& actors = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		Q_FOREACH (auto actor, actors)
			connect (actor,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pierre, LeechCraft::Pierre::Plugin);
