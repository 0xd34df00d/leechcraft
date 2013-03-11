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

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihaveshortcuts.h>

class QDockWidget;

namespace LeechCraft
{
namespace Util
{
	class ShortcutManager;
}

namespace SB2
{
	class ViewManager;
	class TrayComponent;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveShortcuts
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveShortcuts)

		ICoreProxy_ptr Proxy_;

		struct WindowInfo
		{
			ViewManager *Mgr_;
			TrayComponent *Tray_;
		};
		QList<WindowInfo> Managers_;

		Util::ShortcutManager *ShortcutMgr_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		QMap<QString, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QString&, const QKeySequences_t&);
	public slots:
		void hookDockWidgetActionVisToggled (LeechCraft::IHookProxy_ptr,
				QMainWindow*, QDockWidget*, bool);
	private slots:
		void handleWindow (int, bool init = false);
		void handleWindowRemoved (int);
	signals:
		void pluginsAvailable ();
	};
}
}

