/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SIDEBAR_SIDEBAR_H
#define PLUGINS_SIDEBAR_SIDEBAR_H
#include <QObject>
#include <QIcon>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

namespace LeechCraft
{
namespace Sidebar
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		ICoreProxy_ptr Proxy_;

		QToolBar *Bar_;
		QAction *SepAfterPlugins_;
		QAction *SepAfterTabs_;
		QAction *SepAfterQL_;

		QMap<QWidget*, QAction*> TabActions_;

		bool ActionUpdateScheduled_;
		QMap<QAction*, QString> ActionTextUpdates_;
		QMap<QAction*, QIcon> ActionIconUpdates_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	private:
		void UpdateActionPosition (QAction*);
		void ScheduleUpdate ();
	public slots:
		void hookGonnaFillQuickLaunch (LeechCraft::IHookProxy_ptr);
	private slots:
		void handleUpdates ();
		void handleNewTab (const QString&, QWidget*);
		void handleChangeTabName (QWidget*, const QString&);
		void handleChangeTabIcon (QWidget*, const QIcon&);
		void handleRemoveTab (QWidget*);
		void openNewTab ();
	};
}
}

#endif
