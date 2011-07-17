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

#include "metacontacts.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include <interfaces/iclentry.h>
#include "metaprotocol.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_metacontacts");
		
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
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return result;
	}
	
	QObject* Plugin::GetObject ()
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
		if (!entry || entry->GetEntryType () != ICLEntry::ETChat)
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

Q_EXPORT_PLUGIN2 (leechcraft_azoth_metacontacts, LeechCraft::Azoth::Metacontacts::Plugin);
