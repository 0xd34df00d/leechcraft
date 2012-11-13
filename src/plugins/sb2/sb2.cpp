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

#include "sb2.h"
#include <QIcon>
#include <QMainWindow>
#include <QStatusBar>
#include <QGraphicsEffect>
#include <QtDeclarative>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/imwproxy.h>
#include "viewmanager.h"
#include "sbview.h"
#include "launchercomponent.h"
#include "traycomponent.h"
#include "lcmenucomponent.h"

namespace LeechCraft
{
namespace SB2
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		qmlRegisterType<QGraphicsBlurEffect> ("Effects", 1, 0, "Blur");
		qmlRegisterType<QGraphicsColorizeEffect> ("Effects", 1, 0, "Colorize");
		qmlRegisterType<QGraphicsDropShadowEffect> ("Effects", 1, 0, "DropShadow");
		qmlRegisterType<QGraphicsOpacityEffect> ("Effects", 1, 0, "OpacityEffect");

		Mgr_ = new ViewManager (proxy, this);
		auto view = Mgr_->GetView ();
		proxy->GetMWProxy ()->AddSideWidget (view);

		proxy->GetMainWindow ()->statusBar ()->hide ();

		Mgr_->AddComponent ((new LCMenuComponent (proxy))->GetComponent ());

		auto launcher = new LauncherComponent (proxy);
		Mgr_->AddComponent (launcher->GetComponent ());
		connect (this,
				SIGNAL (pluginsAvailable ()),
				launcher,
				SLOT (handlePluginsAvailable ()));

		Tray_ = new TrayComponent (proxy);
		Mgr_->AddComponent (Tray_->GetComponent ());
		connect (this,
				SIGNAL (pluginsAvailable ()),
				Tray_,
				SLOT (handlePluginsAvailable ()));
	}

	void Plugin::SecondInit ()
	{
		emit pluginsAvailable ();

		Mgr_->SecondInit ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SB2";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "SB2";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Next-generation fluid sidebar.");
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

	void Plugin::hookDockWidgetActionVisToggled (IHookProxy_ptr proxy, QDockWidget *dw, bool visible)
	{
		Tray_->HandleDock (dw, visible);
		proxy->CancelDefault ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_sb2, LeechCraft::SB2::Plugin);

