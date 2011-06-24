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

#include "depester.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <plugininterface/util.h>
#include <interfaces/iclentry.h>
#include <interfaces/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Depester
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_depester"));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Depester";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Depester";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows to block messages from unwanted contacts in MUCs.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}
	
	bool Plugin::IsEntryIgnored (QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry ||
				entry->GetEntryType () != ICLEntry::ETPrivateChat)
			return false;
		
		return IgnoredNicks_.contains (entry->GetEntryName ());
	}
	
	void Plugin::HandleMsgOccurence (IHookProxy_ptr proxy, QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (IsEntryIgnored (msg->OtherPart ()))
			proxy->CancelDefault ();
	}
	
	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
		if (!action->property ("Azoth/Depester/IsGood").toBool ())
			return;
		
		QStringList ours;
		ours << "contactListContextMenu";
		proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
	}
	
	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (entry->GetEntryType () != ICLEntry::ETPrivateChat)
			return;
		
		if (!Entry2ActionIgnore_.contains (entryObj))
		{
			QAction *action = new QAction (tr ("Ignore"), entryObj);
			action->setProperty ("Azoth/Depester/IsGood", true);
			action->setProperty ("Azoth/Depester/Entry",
					QVariant::fromValue<QObject*> (entryObj));
			action->setCheckable (true);
			action->setChecked (IsEntryIgnored (entryObj));
			connect (action,
					SIGNAL (toggled (bool)),
					this,
					SLOT (handleIgnoreEntry (bool)));
			Entry2ActionIgnore_ [entryObj] = action;
		}
		QList<QVariant> list = proxy->GetReturnValue ().toList ();
		list << QVariant::fromValue<QObject*> (Entry2ActionIgnore_ [entryObj]);
		proxy->SetReturnValue (list);
	}
	
	void Plugin::hookGonnaAppendMsg (LeechCraft::IHookProxy_ptr proxy,
				QObject *message)
	{
		HandleMsgOccurence (proxy, message);
	}

	void Plugin::hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message)
	{
		HandleMsgOccurence (proxy, message);
	}
	
	void Plugin::handleIgnoreEntry (bool ignore)
	{
		QObject *entryObj = sender ()->
				property ("Azoth/Depester/Entry").value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
			return;
		
		if (ignore)
			IgnoredNicks_ << entry->GetEntryName ();
		else
			IgnoredNicks_.remove (entry->GetEntryName ());
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_depester, LeechCraft::Azoth::Depester::Plugin);
