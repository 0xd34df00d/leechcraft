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

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_METACONTACTS_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_METACONTACTS_H
#include <QObject>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iprotocolplugin.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	class MetaProtocol;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LeechCraft::Azoth::IProtocolPlugin)
		
		MetaProtocol *Proto_;
		QAction *AddToMetacontacts_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
		
		QObject* GetObject ();
		QList<QObject*> GetProtocols () const;
	public slots:
		void hookAddingCLEntryBegin (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
	private slots:
		void handleAddToMetacontacts ();
	};
}
}
}

#endif
