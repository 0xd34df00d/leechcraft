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

#include "otroid.h"
#include <cstring>
#include <QCoreApplication>
#include <QIcon>
#include <QAction>
#include <QTranslator>

extern "C"
{
#include <libotr/version.h>
#include <libotr/privkey.h>

#if OTRL_VERSION_MAJOR >= 4
#include <libotr/instag.h>
#endif
}

#if OTRL_VERSION_MAJOR >= 4
#include <QTimer>
#endif

#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <util/util.h>

namespace LeechCraft
{
namespace Azoth
{
namespace OTRoid
{
	namespace OTR { int IsLoggedIn (void *opData, const char *accName,
				const char*, const char *recipient)
		{
			Plugin *p = static_cast<Plugin*> (opData);
			return p->IsLoggedIn (QString::fromUtf8 (accName),
					QString::fromUtf8 (recipient));
		}

		void InjectMessage (void *opData, const char *accName,
				const char*, const char *recipient, const char *msg)
		{
			Plugin *p = static_cast<Plugin*> (opData);
			p->InjectMsg (QString::fromUtf8 (accName),
					QString::fromUtf8 (recipient),
					QString::fromUtf8 (msg));
		}

		void WriteFingerprints (void *opData)
		{
			static_cast<Plugin*> (opData)->WriteFingerprints ();
		}

		const char* GetAccountName (void *opData, const char *acc, const char*)
		{
			const QString& name = static_cast<Plugin*> (opData)->
					GetAccountName (QString::fromUtf8 (acc));

			const char *orig = name.toUtf8 ().constData ();
			char *result = new char [std::strlen (orig)];
			std::strncpy (result, orig, std::strlen (orig));
			return result;
		}

		void FreeAccountName (void*, const char *name)
		{
			delete [] name;
		}

#if OTRL_VERSION_MAJOR >= 4
		void HandleMsgEvent (void*, OtrlMessageEvent event,
				ConnContext*, const char *msg, gcry_error_t)
		{
			qDebug () << Q_FUNC_INFO
					<< event
					<< msg;
		}

		void TimerControl (void *opData, unsigned int interval)
		{
			static_cast<Plugin*> (opData)->SetPollTimerInterval (interval);
		}
#else
		void LogMsg (void *opData, const char *msg)
		{
			static_cast<Plugin*> (opData)->LogMsg (QString::fromUtf8 (msg).trimmed ());
		}
#endif
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_otroid");

		OTRL_INIT;

		OtrDir_ = Util::CreateIfNotExists ("azoth/otr/");

		UserState_ = otrl_userstate_create ();

		otrl_privkey_read (UserState_, GetOTRFilename ("privkey"));
		otrl_privkey_read_fingerprints (UserState_,
				GetOTRFilename ("fingerprints"), NULL, NULL);

		memset (&OtrOps_, 0, sizeof (OtrOps_));
		OtrOps_.is_logged_in = &OTR::IsLoggedIn;
		OtrOps_.inject_message = &OTR::InjectMessage;
		OtrOps_.write_fingerprints = &OTR::WriteFingerprints;
		OtrOps_.account_name = &OTR::GetAccountName;
		OtrOps_.account_name_free = &OTR::FreeAccountName;
#if OTRL_VERSION_MAJOR >= 4
		OtrOps_.handle_msg_event = &OTR::HandleMsgEvent;
		OtrOps_.timer_control = &OTR::TimerControl;

		PollTimer_ = new QTimer (this);
		connect (PollTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (pollOTR ()));

		SetPollTimerInterval (otrl_message_poll_get_default_interval (UserState_));
#else
		OtrOps_.log_message = &OTR::LogMsg;
#endif
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.OTRoid";
	}

	void Plugin::Release ()
	{
		otrl_userstate_free (UserState_);
	}

	QString Plugin::GetName () const
	{
		return "Azoth OTRoid";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth OTRoid adds support for Off-the-Record deniable encryption system.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/plugins/azoth/plugins/otroid/resources/images/otroid.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	int Plugin::IsLoggedIn (const QString& accId, const QString& entryId)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		if (!entry)
			return -1;

		return entry->Variants ().isEmpty () ? 0 : 1;
	}

	void Plugin::InjectMsg (const QString& accId,
			const QString& entryId, const QString& body)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		if (!entry)
			return;

		QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage,
				QString (), body);
		msgObj->setProperty ("Azoth/HiddenMessage", true);

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
			return;

		msg->Send ();
	}

	void Plugin::Notify (const QString&, const QString&,
			Priority prio, const QString& title,
			const QString& prim, const QString& sec)
	{
		QString text = prim;
		if (!sec.isEmpty ())
			text += "<br />" + sec;

		emit gotEntity (Util::MakeNotification (title, text, prio));
	}

	void Plugin::WriteFingerprints ()
	{
		otrl_privkey_write_fingerprints (UserState_, GetOTRFilename ("fingerprints"));
	}

	QString Plugin::GetAccountName (const QString& accId)
	{
		QObject *accObj = AzothProxy_->GetAccount (accId);
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty account for"
					<< accId
					<< accObj;
			return QString ();
		}

		return acc->GetAccountName ();
	}

#if OTRL_VERSION_MAJOR >= 4
	void Plugin::SetPollTimerInterval (unsigned int seconds)
	{
		if (PollTimer_->isActive ())
			PollTimer_->stop ();

		if (seconds)
			PollTimer_->start (seconds * 1000);
	}
#else
	void Plugin::LogMsg (const QString& msg)
	{
		qDebug () << "OTR:" << msg;
	}
#endif

	void Plugin::initPlugin (QObject *obj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (obj);
	}

	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
		if (!action->property ("Azoth/OTRoid/IsGood").toBool ())
			return;

		QStringList ours;
		ours << "contactListContextMenu"
			<< "tabContextMenu"
			<< "toolbar";

		proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
	}

	void Plugin::hookEntryActionsRemoved (IHookProxy_ptr,
			QObject *entry)
	{
		delete Entry2Action_.take (entry);
	}

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entry)
	{
		if (qobject_cast<ICLEntry*> (entry)->GetEntryType () == ICLEntry::ETMUC)
			return;

		if (!Entry2Action_.contains (entry))
			CreateActions (entry);

		QList<QVariant> list = proxy->GetReturnValue ().toList ();
		list << QVariant::fromValue<QObject*> (Entry2Action_ [entry]);
		proxy->SetReturnValue (list);
	}

	void Plugin::hookGotMessage (IHookProxy_ptr proxy, QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		QObject *entryObj = msg->ParentCLEntry ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry ||
				entry->GetEntryType () == ICLEntry::ETMUC)
			return;

		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		IProtocol *proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		char *newMsg = 0;
		int ignore = otrl_message_receiving (UserState_, &OtrOps_, this,
				acc->GetAccountID ().constData (),
				proto->GetProtocolID ().constData (),
				entry->GetEntryID ().toUtf8 ().constData (),
				msg->GetBody ().toUtf8 ().constData (),
				&newMsg,
				NULL,
				NULL,
#if OTRL_VERSION_MAJOR >= 4
				NULL,
#endif
				NULL);

		if (ignore)
		{
			proxy->CancelDefault ();
			msgObj->setProperty ("Azoth/HiddenMessage", true);
			otrl_message_free (newMsg);
			return;
		}

		if (newMsg)
		{
			msg->SetBody (QString::fromUtf8 (newMsg));
			otrl_message_free (newMsg);

			if (!Entry2Action_.contains (entryObj))
				CreateActions (entryObj);

			Entry2Action_ [entryObj]->setChecked (true);
		}
	}

	void Plugin::hookMessageCreated (IHookProxy_ptr proxy, QObject*, QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		QObject *entryObj = msg->OtherPart ();
		if (!Entry2Action_.contains (entryObj) ||
				!Entry2Action_ [entryObj]->isChecked ())
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		IProtocol *proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		char *newMsg = 0;
		gcry_error_t err = otrl_message_sending (UserState_,
				&OtrOps_,
				this,
				acc->GetAccountID ().constData (),
				proto->GetProtocolID ().constData (),
				entry->GetEntryID ().toUtf8 ().constData (),
#if OTRL_VERSION_MAJOR >= 4
				OTRL_INSTAG_BEST,
#endif
				msg->GetBody ().toUtf8 ().constData (),
				NULL,
				&newMsg,
#if OTRL_VERSION_MAJOR >= 4
				OTRL_FRAGMENT_SEND_SKIP,
				NULL,
#endif
				NULL,
				NULL);

		if (err)
		{
			qWarning () << Q_FUNC_INFO
					<< "OTR error occured, aborting";
			proxy->CancelDefault ();
		}

		if (newMsg)
			msg->SetBody (QString::fromUtf8 (newMsg));

		otrl_message_free (newMsg);
	}

	const char* Plugin::GetOTRFilename (const QString& fname) const
	{
		return OtrDir_.absoluteFilePath (fname).toUtf8 ().constData ();
	}

	void Plugin::CreateActions (QObject *entry)
	{
		QAction *otr = new QAction (tr ("Enable OTR"), this);
		otr->setCheckable (true);
		otr->setIcon (GetIcon ());
		otr->setProperty ("Azoth/OTRoid/IsGood", true);

		Entry2Action_ [entry] = otr;
	}

#if OTRL_VERSION_MAJOR >= 4
	void Plugin::pollOTR ()
	{
		otrl_message_poll (UserState_, &OtrOps_, this);
	}
#endif
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_otroid, LeechCraft::Azoth::OTRoid::Plugin);
