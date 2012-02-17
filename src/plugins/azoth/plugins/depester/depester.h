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

#ifndef PLUGINS_AZOTH_PLUGINS_DEPESTER_DEPESTER_H
#define PLUGINS_AZOTH_PLUGINS_DEPESTER_DEPESTER_H
#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include "core.h"

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace Depester
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		QHash<QObject*, QAction*> Entry2ActionIgnore_;
		QHash<QObject*, QString> Entry2Nick_;
		QSet<QString> IgnoredNicks_;
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
		bool IsEntryIgnored (QObject*);
		void HandleMsgOccurence (IHookProxy_ptr, QObject*);
		void SaveIgnores () const;
		void LoadIgnores ();
	public slots:
		void hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRemoved (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookGonnaAppendMsg (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookShouldCountUnread (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
	private slots:
		void handleIgnoreEntry (bool);
		void handleNameChanged (const QString&);
	};
}
}
}

#endif
