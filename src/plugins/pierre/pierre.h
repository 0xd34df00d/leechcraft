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

#pragma once

#include <QObject>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ientityhandler.h>

class QMenuBar;
class QSystemTrayIcon;

namespace LeechCraft
{
namespace Pierre
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IEntityHandler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IEntityHandler)

		QMenuBar *MenuBar_;
		ICoreProxy_ptr Proxy_;

		QMenu *TrayIconMenu_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);
	public slots:
		void hookGonnaFillMenu (LeechCraft::IHookProxy_ptr);
		void hookTrayIconCreated (LeechCraft::IHookProxy_ptr,
				QSystemTrayIcon*);
		void hookTrayIconVisibilityChanged (LeechCraft::IHookProxy_ptr,
				QSystemTrayIcon*,
				bool);
	private slots:
		void handleGotActions (const QList<QAction*>&, LeechCraft::ActionsEmbedPlace);
		void fillMenu ();
	};
}
}

