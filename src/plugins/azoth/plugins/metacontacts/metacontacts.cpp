/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "metacontacts.h"
#include <QIcon>
#include <QAction>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iclentry.h>
#include "metaprotocol.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Proto_ = new MetaProtocol (this);

		AddToMetacontacts_ = new QAction (tr ("Add to a metacontact..."), this);
		connect (AddToMetacontacts_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddToMetacontacts ()));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Metacontacts";
	}

	void Plugin::Release ()
	{
		Proto_->Release ();
	}

	QString Plugin::GetName () const
	{
		return "Azoth Metacontacts";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth Metacontacts provides support for joining different contacts into one metacontact.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return result;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		QList<QObject*> result;
		result << Proto_;
		return result;
	}

	void Plugin::hookAddingCLEntryBegin (IHookProxy_ptr proxy, QObject *entry)
	{
		if (Core::Instance ().HandleRealEntryAddBegin (entry))
			proxy->CancelDefault ();
	}

	void Plugin::hookDnDEntry2Entry (IHookProxy_ptr proxy,
			QObject *source, QObject *target)
	{
		if (Core::Instance ().HandleDnDEntry2Entry (source, target))
			proxy->CancelDefault ();
	}

	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
		if (action != AddToMetacontacts_)
			return;

		const QStringList& oldList = proxy->GetReturnValue ().toStringList ();
		proxy->SetReturnValue (oldList + QStringList ("contactListContextMenu"));
	}

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry || entry->GetEntryType () != ICLEntry::EntryType::Chat)
			return;

		QList<QVariant> list = proxy->GetReturnValue ().toList ();
		list << QVariant::fromValue<QObject*> (AddToMetacontacts_);
		proxy->SetReturnValue (list);

		AddToMetacontacts_->setProperty ("Azoth/Metacontacts/Object",
				QVariant::fromValue<QObject*> (entryObj));
	}

	void Plugin::handleAddToMetacontacts ()
	{
		QObject *entryObj = sender ()->
				property ("Azoth/Metacontacts/Object").value<QObject*> ();
		if (!entryObj)
		{
			qWarning () << Q_FUNC_INFO
					<< "no corresponding property for sender"
					<< sender ();
			return;
		}

		Core::Instance ().AddRealEntry (entryObj);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_metacontacts, LC::Azoth::Metacontacts::Plugin);
