/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sb2.h"
#include <QIcon>
#include <QMainWindow>
#include <QStatusBar>
#include <QGraphicsEffect>
#include <QtQuick>
#include <QtDebug>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/imwproxy.h>
#include "viewmanager.h"
#include "sbview.h"
#include "launchercomponent.h"
#include "traycomponent.h"
#include "lcmenucomponent.h"
#include "desaturateeffect.h"
#include "dockactioncomponent.h"

Q_DECLARE_METATYPE (QSet<QByteArray>);

namespace LC::SB2
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator (QStringLiteral ("sb2"));
		Proxy_ = proxy;

		ShortcutMgr_ = new Util::ShortcutManager (proxy, this);
		ShortcutMgr_->SetObject (this);

		qmlRegisterType<QGraphicsBlurEffect> ("Effects", 1, 0, "Blur");
		qmlRegisterType<QGraphicsColorizeEffect> ("Effects", 1, 0, "Colorize");
		qmlRegisterType<QGraphicsDropShadowEffect> ("Effects", 1, 0, "DropShadow");
		qmlRegisterType<QGraphicsOpacityEffect> ("Effects", 1, 0, "OpacityEffect");
		qmlRegisterType<DesaturateEffect> ("Effects", 1, 0, "Desaturate");

		qRegisterMetaType<QSet<QByteArray>> ("QSet<QByteArray>");
		qRegisterMetaTypeStreamOperators<QSet<QByteArray>> ();

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
		return QStringLiteral ("SB2");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Side panel with quarks like tab launcher and switcher or tray area, where other plugins can also embed quarks .");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Core.Plugins/1.0" };
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		ShortcutMgr_->SetShortcut (id, seqs);
	}

	void Plugin::hookGonnaShowStatusBar (const IHookProxy_ptr& proxy, bool show)
	{
		if (show)
			proxy->CancelDefault ();
	}

	void Plugin::hookDockWidgetActionVisToggled (const IHookProxy_ptr& proxy,
			QMainWindow*, QDockWidget*, bool)
	{
		proxy->CancelDefault ();
	}

	void Plugin::hookAddingDockAction (const IHookProxy_ptr&,
			QMainWindow *win, QAction *act, Qt::DockWidgetArea)
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		const int idx = rootWM->GetWindowIndex (win);

		Managers_ [idx].Dock_->AddActions ({ act }, TrayComponent::ActionPos::Beginning);
	}

	void Plugin::hookRemovingDockAction (const IHookProxy_ptr&,
			QMainWindow *win, QAction *act, Qt::DockWidgetArea)
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		const int idx = rootWM->GetWindowIndex (win);

		Managers_ [idx].Dock_->RemoveAction (act);
	}

	void Plugin::hookDockBarWillBeShown (const IHookProxy_ptr& proxy,
			QMainWindow*, QToolBar*, Qt::DockWidgetArea)
	{
		proxy->CancelDefault ();
	}

	void Plugin::handleWindow (int index, bool init)
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		auto win = rootWM->GetMainWindow (index);

		auto mgr = std::make_shared<ViewManager> (Proxy_, ShortcutMgr_, win, this);
		auto view = mgr->GetView ();

		auto mwProxy = rootWM->GetMWProxy (index);
		auto ictw = rootWM->GetTabWidget (index);

		win->statusBar ()->hide ();

		mgr->RegisterInternalComponent ((new LCMenuComponent (mwProxy))->GetComponent ());

		auto launcher = std::make_shared<LauncherComponent> (ictw, Proxy_, mgr.get ());
		mgr->RegisterInternalComponent (launcher->GetComponent ());
		if (init)
			connect (this,
					&Plugin::pluginsAvailable,
					launcher.get (),
					&LauncherComponent::handlePluginsAvailable);
		else
			launcher->handlePluginsAvailable ();

		auto tray = std::make_shared<TrayComponent> (Proxy_, view);
		mgr->RegisterInternalComponent (tray->GetComponent ());
		if (init)
			connect (this,
					&Plugin::pluginsAvailable,
					tray.get (),
					&TrayComponent::handlePluginsAvailable);
		else
			tray->handlePluginsAvailable ();

		auto dock = std::make_shared<DockActionComponent> (Proxy_, view);
		mgr->RegisterInternalComponent (dock->GetComponent ());

		if (!init)
			mgr->SecondInit ();

		Managers_.push_back ({ mgr, tray, launcher, dock });
	}

	void Plugin::handleWindowRemoved (int index)
	{
		Managers_.removeAt (index);
	}
}

LC_EXPORT_PLUGIN (leechcraft_sb2, LC::SB2::Plugin);

