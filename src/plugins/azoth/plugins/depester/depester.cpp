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

#include "depester.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>

Q_DECLARE_METATYPE (QSet<QString>);

namespace LeechCraft
{
namespace Azoth
{
namespace Depester
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_depester");
		qRegisterMetaType<QSet<QString>> ("QSet<QString>");
		qRegisterMetaTypeStreamOperators<QSet<QString>> ("QSet<QString>");

		LoadIgnores ();
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
		return tr ("Allows one to block messages from unwanted contacts in MUCs.");
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

	void Plugin::SaveIgnores () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Depester");
		settings.setValue ("IgnoredNicks", QVariant::fromValue (IgnoredNicks_));
	}

	void Plugin::LoadIgnores ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Depester");
		IgnoredNicks_ = settings.value ("IgnoredNicks").value<QSet<QString>> ();
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

	void Plugin::hookEntryActionsRemoved (IHookProxy_ptr, QObject *entry)
	{
		delete Entry2ActionIgnore_.take (entry);
		Entry2Nick_.remove (entry);
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

	void Plugin::hookShouldCountUnread (IHookProxy_ptr proxy,
				QObject *message)
	{
		IMessage *msg = qobject_cast<IMessage*> (message);
		if (IsEntryIgnored (msg->OtherPart ()))
		{
			proxy->CancelDefault ();
			proxy->SetReturnValue (false);
		}
	}

	void Plugin::handleIgnoreEntry (bool ignore)
	{
		QObject *entryObj = sender ()->
				property ("Azoth/Depester/Entry").value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
			return;

		if (ignore)
		{
			const QString& nick = entry->GetEntryName ();
			IgnoredNicks_ << nick;
			Entry2Nick_ [entryObj] = nick;
			connect (entryObj,
					SIGNAL (nameChanged (const QString&)),
					this,
					SLOT (handleNameChanged (const QString&)));
		}
		else
		{
			IgnoredNicks_.remove (entry->GetEntryName ());
			Entry2Nick_.remove (entryObj);
			disconnect (entryObj,
					SIGNAL (nameChanged (const QString&)),
					this,
					SLOT (handleNameChanged (const QString&)));
		}

		SaveIgnores ();
	}

	void Plugin::handleNameChanged (const QString& name)
	{
		QObject *entryObj = sender ();
		if (!entryObj)
			return;

		IgnoredNicks_.remove (Entry2Nick_ [entryObj]);
		IgnoredNicks_ << name;
		Entry2Nick_ [entryObj] = name;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_depester, LeechCraft::Azoth::Depester::Plugin);
