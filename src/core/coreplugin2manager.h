/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkAccessManager>
#include <QMainWindow>
#include <QAction>
#include "util/xpc/basehookinterconnector.h"
#include "interfaces/core/ihookproxy.h"
#include "interfaces/iinfo.h"

class QMenu;
class QDockWidget;
class QSystemTrayIcon;

namespace LC
{
	class CorePlugin2Manager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		CorePlugin2Manager (QObject* = 0);
	signals:
		void hookDockWidgetActionVisToggled (LC::IHookProxy_ptr proxy,
				QMainWindow *window,
				QDockWidget *dock,
				bool toggleActionVisible);

		void hookAddingDockAction (LC::IHookProxy_ptr, QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookRemovingDockAction (LC::IHookProxy_ptr, QMainWindow*, QAction*, Qt::DockWidgetArea);

		void hookDockBarWillBeShown (LC::IHookProxy_ptr, QMainWindow*, QToolBar*, Qt::DockWidgetArea);

		void hookGonnaFillMenu (LC::IHookProxy_ptr);
		void hookGonnaFillQuickLaunch (LC::IHookProxy_ptr proxy);
		void hookGonnaShowStatusBar (LC::IHookProxy_ptr, bool);
		void hookNAMCreateRequest (LC::IHookProxy_ptr proxy,
				QNetworkAccessManager *manager,
				QNetworkAccessManager::Operation *op,
				QIODevice **dev);
		void hookTabContextMenuFill (LC::IHookProxy_ptr proxy,
				QMenu *menu, int index, int windowId);

		void hookTabAdding (LC::IHookProxy_ptr proxy,
				QWidget *tabWidget);
		void hookTabFinishedMoving (LC::IHookProxy_ptr proxy,
				int index, int windowId);
		void hookTabSetText (LC::IHookProxy_ptr proxy,
				int index, int windowId);
		void hookTabIsRemoving (LC::IHookProxy_ptr proxy,
				int index,
				int windowId);

		void hookGetPreferredWindowIndex (LC::IHookProxy_ptr proxy,
				const QWidget *widget);
		void hookGetPreferredWindowIndex (LC::IHookProxy_ptr proxy,
				const QByteArray& tabClass);
		void hookGetPreferredWindowIndex (LC::IHookProxy_ptr proxy);

		void hookTrayIconCreated (LC::IHookProxy_ptr,
				QSystemTrayIcon*);
		void hookTrayIconVisibilityChanged (LC::IHookProxy_ptr,
				QSystemTrayIcon*,
				bool);
	};
}
