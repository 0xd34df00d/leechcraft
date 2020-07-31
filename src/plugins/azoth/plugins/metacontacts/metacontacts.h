/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_METACONTACTS_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_METACONTACTS_H
#include <QObject>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iprotocolplugin.h>

class QAction;

namespace LC
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
		Q_INTERFACES (IInfo IPlugin2 LC::Azoth::IProtocolPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.MetaContacts")

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

		QObject* GetQObject ();
		QList<QObject*> GetProtocols () const;
	public slots:
		void hookAddingCLEntryBegin (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookDnDEntry2Entry (LC::IHookProxy_ptr,
				QObject*, QObject*);
		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entry);
	private slots:
		void handleAddToMetacontacts ();
	signals:
		void gotNewProtocols (const QList<QObject*>&);
	};
}
}
}

#endif
