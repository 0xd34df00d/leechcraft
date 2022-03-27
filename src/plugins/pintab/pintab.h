/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMenu>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QMainWindow;

namespace LC
{
namespace PinTab
{
	class Plugin : public QObject
				, public IInfo
				, public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.PinTab")

		ICoreTabWidget *MainTabWidget_;

		ICoreProxy_ptr Proxy_;

		QHash<QMainWindow*, QHash<QWidget*, QPair<QString, QWidget*>>> Window2Widget2TabData_;
		QTabBar::ButtonPosition CloseSide_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	private:
		void PinTab (QWidget *tab, int windowIndex);
		void UnPinTab (QWidget *tab, int windowIndex);
	public slots:
		void hookTabContextMenuFill (LC::IHookProxy_ptr proxy,
				QMenu *menu, int index, int windowId);
		void hookTabFinishedMoving (LC::IHookProxy_ptr proxy, int index,
				int windowId);
		void hookTabSetText (LC::IHookProxy_ptr proxy, int index,
				int windowId);
	private slots:
		void checkPinState (int windowId, QWidget *tab);
		void handleTabRemoving (int windowId, QWidget *tab);

		void handleWindowRemoved (int index);
	};
}
}
