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

#include "core.h"
#include <QMetaObject>
#include <QVariant>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/imessage.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/iclentry.h>
#include <interfaces/iaccount.h>
#include "storage.h"
#include "storagethread.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	boost::weak_ptr<Core> Core::InstPtr_;

	Core::Core ()
	: StorageThread_ (new StorageThread ())
	, PluginProxy_ (0)
	{
		StorageThread_->start (QThread::LowestPriority);

		TabClass_.TabClass_ = "Chathistory";
		TabClass_.VisibleName_ = tr ("Chat history");
		TabClass_.Description_ = tr ("Chat history viewer for the Azoth IM");
		TabClass_.Priority_ = 40;
		TabClass_.Features_ = TFOpenableByRequest;

		LoadDisabled ();
	}

	boost::shared_ptr<Core> Core::Instance ()
	{
		if (InstPtr_.expired ())
		{
			boost::shared_ptr<Core> ptr (new Core);
			InstPtr_ = ptr;
			return ptr;
		}
		return InstPtr_.lock ();
	}

	Core::~Core ()
	{
		StorageThread_->quit ();
		StorageThread_->wait (2000);

		if (StorageThread_->isRunning ())
		{
			qWarning () << Q_FUNC_INFO
					<< "storage thread still running, forcefully terminating...";
			StorageThread_->terminate ();
			StorageThread_->wait (5000);
		}
		else
			delete StorageThread_;
	}

	TabClassInfo Core::GetTabClass () const
	{
		return TabClass_;
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	IProxyObject* Core::GetPluginProxy () const
	{
		return PluginProxy_;
	}

	bool Core::IsLoggingEnabled (QObject *entryObj) const
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "could not be casted to ICLEntry";
			return true;
		}

		return !DisabledIDs_.contains (entry->GetEntryID ());
	}

	void Core::SetLoggingEnabled (QObject *entryObj, bool enable)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "could not be casted to ICLEntry";
			return;
		}

		const QString& id = entry->GetEntryID ();
		if (enable)
			DisabledIDs_.remove (id);
		else
			DisabledIDs_ << id;

		SaveDisabled ();
	}

	void Core::Process (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (msg->GetMessageType () != IMessage::MTChatMessage &&
			msg->GetMessageType () != IMessage::MTMUCMessage)
			return;
		if (msg->GetBody ().isEmpty ())
			return;
		if (msg->GetDirection () == IMessage::DOut &&
				msg->GetMessageType () == IMessage::MTMUCMessage)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part doesn't implement ICLEntry"
					<< msg->GetObject ()
					<< msg->OtherPart ();
			return;
		}
		if (DisabledIDs_.contains (entry->GetEntryID ()))
			return;

		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "message's account doesn't implement IAccount"
					<< entry->GetParentAccount ();
			return;
		}

		QVariantMap data;
		data ["EntryID"] = entry->GetEntryID ();
		data ["VisibleName"] = entry->GetEntryName ();
		data ["AccountID"] = acc->GetAccountID ();
		data ["DateTime"] = msg->GetDateTime ();
		data ["Direction"] = msg->GetDirection () == IMessage::DIn ? "IN" : "OUT";
		data ["Body"] = msg->GetBody ();
		data ["OtherVariant"] = msg->GetOtherVariant ();
		data ["MessageType"] = static_cast<int> (msg->GetMessageType ());

		QMetaObject::invokeMethod (StorageThread_->GetStorage (),
				"addMessage",
				Qt::QueuedConnection,
				Q_ARG (QVariantMap, data));
	}

	void Core::GetOurAccounts ()
	{
		QMetaObject::invokeMethod (StorageThread_->GetStorage (),
				"getOurAccounts",
				Qt::QueuedConnection);
	}

	void Core::GetUsersForAccount (const QString& accountID)
	{
		QMetaObject::invokeMethod (StorageThread_->GetStorage (),
				"getUsersForAccount",
				Qt::QueuedConnection,
				Q_ARG (QString, accountID));
	}

	void Core::GetChatLogs (const QString& accountId,
			const QString& entryId, int backpages, int amount)
	{
		QMetaObject::invokeMethod (StorageThread_->GetStorage (),
				"getChatLogs",
				Qt::QueuedConnection,
				Q_ARG (QString, accountId),
				Q_ARG (QString, entryId),
				Q_ARG (int, backpages),
				Q_ARG (int, amount));
	}

	void Core::Search (const QString& accountId, const QString& entryId,
			const QString& text, int shift)
	{
		QMetaObject::invokeMethod (StorageThread_->GetStorage (),
				"search",
				Qt::QueuedConnection,
				Q_ARG (QString, accountId),
				Q_ARG (QString, entryId),
				Q_ARG (QString, text),
				Q_ARG (int, shift));
	}

	void Core::ClearHistory (const QString& accountId, const QString& entryId)
	{
		QMetaObject::invokeMethod (StorageThread_->GetStorage (),
				"clearHistory",
				Qt::QueuedConnection,
				Q_ARG (QString, accountId),
				Q_ARG (QString, entryId));
	}

	void Core::LoadDisabled ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_ChatHistory");
		DisabledIDs_ = settings.value ("DisabledIDs").toStringList ().toSet ();
	}

	void Core::SaveDisabled ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_ChatHistory");
		settings.setValue ("DisabledIDs", QStringList (DisabledIDs_.toList ()));
	}
}
}
}
