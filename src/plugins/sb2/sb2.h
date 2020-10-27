/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMainWindow>
#include <QToolBar>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihaveshortcuts.h>

class QDockWidget;

namespace LC::Util
	{
		class ShortcutManager;
	}

namespace LC::SB2
{
	class ViewManager;
	class TrayComponent;
	class LauncherComponent;
	class DockActionComponent;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveShortcuts)

		LC_PLUGIN_METADATA ("org.LeechCraft.SB2")

		ICoreProxy_ptr Proxy_;

		struct WindowInfo
		{
			std::shared_ptr<ViewManager> Mgr_;
			std::shared_ptr<TrayComponent> Tray_;
			std::shared_ptr<LauncherComponent> Launcher_;
			std::shared_ptr<DockActionComponent> Dock_;
		};
		QList<WindowInfo> Managers_;

		Util::ShortcutManager *ShortcutMgr_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QMap<QString, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QString&, const QKeySequences_t&) override;
	public slots:
		void hookGonnaShowStatusBar (LC::IHookProxy_ptr, bool);
		void hookDockWidgetActionVisToggled (LC::IHookProxy_ptr,
				QMainWindow*, QDockWidget*, bool);
		void hookAddingDockAction (LC::IHookProxy_ptr,
				QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookRemovingDockAction (LC::IHookProxy_ptr,
				QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookDockBarWillBeShown (LC::IHookProxy_ptr,
				QMainWindow*, QToolBar*, Qt::DockWidgetArea);
	private slots:
		void handleWindow (int, bool init = false);
		void handleWindowRemoved (int);
	signals:
		void pluginsAvailable ();
	};
}

