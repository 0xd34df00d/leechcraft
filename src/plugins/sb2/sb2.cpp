/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/imwproxy.h>
#include "viewmanager.h"
#include "sbview.h"
#include "launchercomponent.h"
#include "traycomponent.h"
#include "lcmenucomponent.h"
#include "desaturateeffect.h"

namespace LeechCraft
{
namespace SB2
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		qmlRegisterType<QGraphicsBlurEffect> ("Effects", 1, 0, "Blur");
		qmlRegisterType<QGraphicsColorizeEffect> ("Effects", 1, 0, "Colorize");
		qmlRegisterType<QGraphicsDropShadowEffect> ("Effects", 1, 0, "DropShadow");
		qmlRegisterType<QGraphicsOpacityEffect> ("Effects", 1, 0, "OpacityEffect");
		qmlRegisterType<DesaturateEffect> ("Effects", 1, 0, "Desaturate");

		auto rootWM = proxy->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i, true);

		connect (rootWM->GetObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
		connect (rootWM->GetObject (),
				SIGNAL (windowRemoved (int)),
				this,
				SLOT (handleWindowRemoved (int)));
	}

	void Plugin::SecondInit ()
	{
		emit pluginsAvailable ();

		for (const auto& info : Managers_)
			info.Mgr_->SecondInit ();
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

	void Plugin::hookDockWidgetActionVisToggled (IHookProxy_ptr proxy,
			QMainWindow *win, QDockWidget *dw, bool visible)
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		const int idx = rootWM->GetWindowIndex (win);

		Managers_ [idx].Tray_->HandleDock (dw, visible);
		proxy->CancelDefault ();
	}

	void Plugin::handleWindow (int index, bool init)
	{
		auto mgr = new ViewManager (Proxy_, this);
		auto view = mgr->GetView ();

		auto rootWM = Proxy_->GetRootWindowsManager ();
		auto mwProxy = rootWM->GetMWProxy (index);
		auto ictw = rootWM->GetTabWidget (index);
		mwProxy->AddSideWidget (view);
		rootWM->GetMainWindow (index)->statusBar ()->hide ();

		mgr->AddComponent ((new LCMenuComponent (mwProxy))->GetComponent ());

		auto launcher = new LauncherComponent (ictw, Proxy_);
		mgr->AddComponent (launcher->GetComponent ());
		if (init)
			connect (this,
					SIGNAL (pluginsAvailable ()),
					launcher,
					SLOT (handlePluginsAvailable ()));
		else
			launcher->handlePluginsAvailable ();

		auto tray = new TrayComponent (Proxy_);
		mgr->AddComponent (tray->GetComponent ());
		if (init)
			connect (this,
					SIGNAL (pluginsAvailable ()),
					tray,
					SLOT (handlePluginsAvailable ()));
		else
			tray->handlePluginsAvailable ();

		if (!init)
			mgr->SecondInit ();

		Managers_.push_back ({ mgr, tray });
	}

	void Plugin::handleWindowRemoved (int index)
	{
		const auto& info = Managers_.takeAt (index);
		delete info.Mgr_;
		delete info.Tray_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_sb2, LeechCraft::SB2::Plugin);

