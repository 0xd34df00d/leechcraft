/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#ifndef LEECHCRAFT_PINTAB_PINTAB_H
#define LEECHCRAFT_PINTAB_PINTAB_H

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QMenu;

namespace LeechCraft
{
namespace PinTab
{
	class Plugin : public QObject
				, public IInfo
				, public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		ICoreTabWidget *MainTabWidget_;
		QAction *PinTab_;
		QAction *UnPinTab_;

		int Id_;

		QHash<int, QPair<QString, QWidget*>> PinTabsIndex2TabData_;
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
	public slots:
		void hookTabContextMenuFill (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu, int index);
		void hookTabFinishedMoving (LeechCraft::IHookProxy_ptr proxy, int index);
		void hookTabSetText (LeechCraft::IHookProxy_ptr proxy, int index);
	private slots:
		void pinTab (int index = -1);
		void unPinTab (int index = -1);
		void checkPinState (int index);
	};
}
}

#endif // LEECHCRAFT_PINTAB_PINTAB_H
