/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <util/shortcuts/shortcutmanager.h>
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

		ShortcutMgr_ = new Util::ShortcutManager (proxy, this);
		ShortcutMgr_->SetObject (this);

		qmlRegisterType<QGraphicsBlurEffect> ("Effects", 1, 0, "Blur");
		qmlRegisterType<QGraphicsColorizeEffect> ("Effects", 1, 0, "Colorize");
		qmlRegisterType<QGraphicsDropShadowEffect> ("Effects", 1, 0, "DropShadow");
		qmlRegisterType<QGraphicsOpacityEffect> ("Effects", 1, 0, "OpacityEffect");
		qmlRegisterType<DesaturateEffect> ("Effects", 1, 0, "Desaturate");

		auto rootWM = proxy->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i, true);

		connect (rootWM->GetQObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
		connect (rootWM->GetQObject (),
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
		static QIcon icon (":/resources/images/sb2.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		ShortcutMgr_->SetShortcut (id, seqs);
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
		auto rootWM = Proxy_->GetRootWindowsManager ();
		auto win = rootWM->GetMainWindow (index);

		auto mgr = new ViewManager (Proxy_, ShortcutMgr_, win, this);
		auto view = mgr->GetView ();

		auto mwProxy = rootWM->GetMWProxy (index);
		auto ictw = rootWM->GetTabWidget (index);

		win->statusBar ()->hide ();

		mgr->RegisterInternalComponent ((new LCMenuComponent (mwProxy))->GetComponent ());

		auto launcher = new LauncherComponent (ictw, Proxy_, mgr);
		mgr->RegisterInternalComponent (launcher->GetComponent ());
		if (init)
			connect (this,
					SIGNAL (pluginsAvailable ()),
					launcher,
					SLOT (handlePluginsAvailable ()));
		else
			launcher->handlePluginsAvailable ();

		auto tray = new TrayComponent (Proxy_, view);
		mgr->RegisterInternalComponent (tray->GetComponent ());
		if (init)
			connect (this,
					SIGNAL (pluginsAvailable ()),
					tray,
					SLOT (handlePluginsAvailable ()));
		else
		{
			tray->handlePluginsAvailable ();
			mgr->SecondInit ();
		}

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

