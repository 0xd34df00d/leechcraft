/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "otrhandler.h"
#include <cstring>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include <QMenu>
#include <QMessageBox>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTimer>

extern "C"
{
#include <libotr/version.h>
#include <libotr/privkey.h>
#include <libotr/message.h>
#include <libotr/proto.h>
#include <libotr/instag.h>
}

#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "util.h"
#include "authenticator.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	namespace OTR
	{
		int IsLoggedIn (void *opData, const char *accName,
				const char*, const char *recipient)
		{
			return static_cast<OtrHandler*> (opData)->IsLoggedIn (QString::fromUtf8 (accName),
					QString::fromUtf8 (recipient));
		}

		void InjectMessage (void *opData, const char *accName,
				const char*, const char *recipient, const char *msg)
		{
			static_cast<OtrHandler*> (opData)->InjectMsg (QString::fromUtf8 (accName),
					QString::fromUtf8 (recipient),
					QString::fromUtf8 (msg),
					true,
					IMessage::Direction::Out);
		}

		void Notify (void *opData, OtrlNotifyLevel level,
				const char *accountname, const char*,
				const char *username, const char *title,
				const char *primary, const char *secondary)
		{
			auto u = [] (const char *cs) { return QString::fromUtf8 (cs); };

			Priority prio = Priority::Info;
			switch (level)
			{
			case OTRL_NOTIFY_ERROR:
				prio = Priority::Critical;
				break;
			case OTRL_NOTIFY_WARNING:
				prio = Priority::Warning;
				break;
			case OTRL_NOTIFY_INFO:
				prio = Priority::Info;
				break;
			}

			static_cast<OtrHandler*> (opData)->Notify (u (accountname),
					u (username), prio, u (title), u (primary), u (secondary));
		}

		void HandleSmpEvent (void *opData, OtrlSMPEvent smpEvent,
				ConnContext *context, unsigned short progressPercent,
				char *question)
		{
			static_cast<OtrHandler*> (opData)->HandleSmpEvent (smpEvent,
					context, progressPercent, QString::fromUtf8 (question));
		}

		void HandleMsgEvent (void *opData, OtrlMessageEvent event,
				ConnContext *context, const char *message, gcry_error_t err)
		{
			qDebug () << Q_FUNC_INFO
					<< event
					<< message;

			auto plugin = static_cast<OtrHandler*> (opData);

			const auto& contact = plugin->GetVisibleEntryName (QString::fromUtf8 (context->accountname),
					QString::fromUtf8 (context->username));

			QString msg;
			switch (event)
			{
			case OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:
				msg = OtrHandler::tr ("The following message received from %1 "
								   "was not encrypted:").arg (contact);
				break;
			case OTRL_MSGEVENT_CONNECTION_ENDED:
				msg = OtrHandler::tr ("Your message was not sent. Either end your "
								   "private conversation, or restart it.");
				break;
			case OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED:
				msg = OtrHandler::tr ("Unreadable encrypted message was received.");
				break;
			case OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE:
				msg = OtrHandler::tr ("Received an encrypted message but it cannot "
								   "be read because no private connection is "
								   "established yet.");
				break;
			case OTRL_MSGEVENT_RCVDMSG_UNREADABLE:
				msg = OtrHandler::tr ("Received message is unreadable.");
				break;
			case OTRL_MSGEVENT_RCVDMSG_MALFORMED:
				msg = OtrHandler::tr ("Received message contains malformed data.");
				break;
			case OTRL_MSGEVENT_ENCRYPTION_ERROR:
				msg = OtrHandler::tr ("OTR encryption error, the message has not been sent.");
				break;
			case OTRL_MSGEVENT_ENCRYPTION_REQUIRED:
				msg = OtrHandler::tr ("Trying to send unencrypted message while our policy requires OTR encryption.");
				break;
			case OTRL_MSGEVENT_SETUP_ERROR:
				msg = OtrHandler::tr ("Private conversation could not be set up. Error %1, source %2.")
						.arg (QString::fromUtf8 (gcry_strerror (err)))
						.arg (QString::fromUtf8 (gcry_strsource (err)));
				break;
			case OTRL_MSGEVENT_MSG_REFLECTED:
				msg = OtrHandler::tr ("Received our own OTR message.");
				break;
			case OTRL_MSGEVENT_MSG_RESENT:
				msg = OtrHandler::tr ("The previous message has been resent.");
				break;
			case OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE:
				msg = OtrHandler::tr ("Received (and discarded) message for other client instance.");
				break;
			case OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR:
				msg = OtrHandler::tr ("Received general OTR error.");
				break;
			case OTRL_MSGEVENT_LOG_HEARTBEAT_RCVD:
			case OTRL_MSGEVENT_LOG_HEARTBEAT_SENT:
			case OTRL_MSGEVENT_NONE:
				break;
			}

			if (!msg.isEmpty ())
			{
				if (message)
					msg += " " + OtrHandler::tr ("Original OTR message: %1.")
							.arg (QString::fromUtf8 (message));

				plugin->InjectMsg (QString::fromUtf8 (context->accountname),
						QString::fromUtf8 (context->username),
						msg, false, IMessage::Direction::In,
						IMessage::Type::ServiceMessage);
			}
		}

		void TimerControl (void *opData, unsigned int interval)
		{
			static_cast<OtrHandler*> (opData)->SetPollTimerInterval (interval);
		}

		void HandleNewFingerprint (void *opData, OtrlUserState,
				const char *accountname, const char*,
				const char *username, unsigned char fingerprint [20])
		{
			char fpHash [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
			otrl_privkey_hash_to_human (fpHash, fingerprint);
			QString hrHash (fpHash); // human readable fingerprint

			const auto plugin = static_cast<OtrHandler*> (opData);

			const auto& msg = OtrHandler::tr ("You have received a new fingerprint from %1: %2")
					.arg (plugin->GetVisibleEntryName (QString::fromUtf8 (accountname), QString::fromUtf8 (username)))
					.arg (hrHash);
			plugin->InjectMsg (QString::fromUtf8 (accountname),
					QString::fromUtf8 (username),
					msg, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
		}

		void HandleGoneSecure (void *opData, ConnContext *context)
		{
			const auto& msg = OtrHandler::tr ("Private conversation started");
			static_cast<OtrHandler*> (opData)->
					InjectMsg (QString::fromUtf8 (context->accountname),
							QString::fromUtf8 (context->username),
							msg, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
		}

		void HandleGoneInsecure (void *opData, ConnContext *context)
		{
			const auto& msg = OtrHandler::tr ("Private conversation lost");
			static_cast<OtrHandler*> (opData)->
					InjectMsg (QString::fromUtf8 (context->accountname),
							QString::fromUtf8 (context->username),
							msg, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
		}

		void HandleStillSecure (void *opData, ConnContext *context, int)
		{
			const auto& msg = QObject::tr ("Private conversation refreshed");
			static_cast<OtrHandler*> (opData)->
					InjectMsg (QString::fromUtf8 (context->accountname),
							QString::fromUtf8 (context->username),
							msg, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
		}
	}

	OtrHandler::OtrHandler (const ICoreProxy_ptr& coreProxy, IProxyObject *azothProxy)
	: CoreProxy_ { coreProxy }
	, AzothProxy_ { azothProxy }
	, OtrDir_ { Util::CreateIfNotExists ("azoth/otr/") }
	, UserState_ { otrl_userstate_create () }
	{
		otrl_privkey_read (UserState_, GetOTRFilename ("privkey").constData ());
		otrl_privkey_read_fingerprints (UserState_,
				GetOTRFilename ("fingerprints").constData (), NULL, NULL);
		otrl_instag_read (UserState_, GetOTRFilename ("instags").constData ());

		memset (&OtrOps_, 0, sizeof (OtrOps_));
		OtrOps_.policy = [] (void*, ConnContext*) { return OtrlPolicy { OTRL_POLICY_DEFAULT }; };
		OtrOps_.create_privkey = [] (void *opData, const char *accName, const char *proto)
				{ static_cast<OtrHandler*> (opData)->CreatePrivkey (accName, proto); };
		OtrOps_.create_instag = [] (void *opData, const char *accName, const char *proto)
				{ static_cast<OtrHandler*> (opData)->CreateInstag (accName, proto); };
		OtrOps_.is_logged_in = &OTR::IsLoggedIn;
		OtrOps_.inject_message = &OTR::InjectMessage;
		OtrOps_.update_context_list = [] (void*) {};
		OtrOps_.new_fingerprint = &OTR::HandleNewFingerprint;
		OtrOps_.write_fingerprints = [] (void *opData)
				{ static_cast<OtrHandler*> (opData)->writeFingerprints (); };
		OtrOps_.account_name = [] (void *opData, const char *acc, const char*) -> const char*
				{
					const auto& name = static_cast<OtrHandler*> (opData)->
							GetAccountName (QString::fromUtf8 (acc)).toUtf8 ();

					const char *orig = name.constData ();
					char *result = static_cast<char*> (malloc (name.size () + 1));
					// QByteArray is guaranteed to have a terminating \0
					std::strncpy (result, orig, name.size () + 1);
					return result;
				};
		OtrOps_.account_name_free = [] (void*, const char *name) { delete [] name; };
		OtrOps_.gone_secure = &OTR::HandleGoneSecure;
		OtrOps_.gone_insecure = &OTR::HandleGoneInsecure;
		OtrOps_.still_secure = &OTR::HandleStillSecure;
		OtrOps_.handle_smp_event = &OTR::HandleSmpEvent;
		OtrOps_.handle_msg_event = &OTR::HandleMsgEvent;
		OtrOps_.timer_control = &OTR::TimerControl;

		PollTimer_ = new QTimer (this);
		connect (PollTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (pollOTR ()));

		SetPollTimerInterval (otrl_message_poll_get_default_interval (UserState_));
	}

	OtrHandler::~OtrHandler ()
	{
		otrl_userstate_free (UserState_);
	}

	namespace
	{
		QString GetVisibleEntryNameImpl (ICLEntry *entry)
		{
			const auto& id = entry->GetHumanReadableID ();
			const auto& name = entry->GetEntryName ();
			return name != id ?
					QString ("%1 (%2)").arg (name).arg (id) :
					id;
		}
	}

	void OtrHandler::HandleMessageCreated (const IHookProxy_ptr& proxy, IMessage *msg)
	{
		if (IsGenerating_)
			return;

		QObject *entryObj = msg->OtherPart ();
		if (!Entry2Action_.contains (entryObj) ||
				!Entry2Action_ [entryObj].ToggleOtr_->isChecked ())
			return;

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto acc = entry->GetParentAccount ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		char *newMsg = 0;
		const auto err = otrl_message_sending (UserState_,
				&OtrOps_,
				this,
				acc->GetAccountID ().constData (),
				proto->GetProtocolID ().constData (),
				entry->GetEntryID ().toUtf8 ().constData (),
				OTRL_INSTAG_BEST,
				msg->GetBody ().toUtf8 ().constData (),
				NULL,
				&newMsg,
				OTRL_FRAGMENT_SEND_SKIP,
				NULL,
				NULL,
				NULL);

		if (err)
		{
			qWarning () << Q_FUNC_INFO
					<< "OTR error occured, aborting";
			proxy->CancelDefault ();
		}

		if (newMsg)
		{
			Msg2OrigText_ [msg->GetQObject ()] = msg->GetBody ();
			msg->SetBody (QString::fromUtf8 (newMsg));
			msg->ToggleOTRMessage (true);
		}

		otrl_message_free (newMsg);
	}

	void OtrHandler::HandleGotMessage (const IHookProxy_ptr& proxy, QObject *msgObj)
	{
		if (IsGenerating_)
			return;

		if (PendingInjectedMessages_.remove (msgObj))
			return;

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		if (msg->IsForwarded ())
		{
			if (msg->GetBody ().startsWith ("?OTR"))
			{
				proxy->CancelDefault ();
				msgObj->setProperty ("Azoth/HiddenMessage", true);
			}
			return;
		}

		if (msg->GetDirection () == IMessage::Direction::Out &&
				Msg2OrigText_.contains (msgObj))
		{
			msg->SetBody (Msg2OrigText_.take (msgObj));
			return;
		}

		if (msg->GetMessageType () != IMessage::Type::ChatMessage ||
			msg->GetDirection () != IMessage::Direction::In)
			return;

		const auto entryObj = msg->ParentCLEntry ();
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry ||
				entry->GetEntryType () == ICLEntry::EntryType::MUC)
			return;

		const auto acc = entry->GetParentAccount ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		char *newMsg = 0;
		OtrlTLV *tlvs = 0;
		int ignore = otrl_message_receiving (UserState_, &OtrOps_, this,
				acc->GetAccountID ().constData (),
				proto->GetProtocolID ().constData (),
				entry->GetEntryID ().toUtf8 ().constData (),
				msg->GetBody ().toUtf8 ().constData (),
				&newMsg,
				&tlvs,
				NULL,
				NULL,
				NULL);

		OtrlTLV *tlv = otrl_tlv_find (tlvs, OTRL_TLV_DISCONNECTED);
		if (tlv)
		{
			const auto& message = tr ("%1 has ended the private conversation with you, "
					"you should do the same.")
						.arg (GetVisibleEntryNameImpl (entry));
			InjectMsg (acc->GetAccountID (), entry->GetEntryID (),
						message, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
		}
		otrl_tlv_free (tlvs);

		// Magic hack to force it work similar to libotr < 4.0.0.
		// If user received unencrypted message he (she) should be notified.
		// See OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED as well.
		if (!msg->GetBody ().startsWith("?OTR") && ignore && !newMsg)
			ignore = 0;

		if (ignore)
		{
			proxy->CancelDefault ();
			msgObj->setProperty ("Azoth/HiddenMessage", true);
		}

		if (newMsg)
		{
			msg->SetBody (QString::fromUtf8 (newMsg));
			otrl_message_free (newMsg);
		}

		if (ignore || newMsg)
		{
			msg->ToggleOTRMessage (true);

			if (!Entry2Action_.contains (entryObj))
				CreateActions (entryObj);

			if (!tlv)
				Entry2Action_ [entryObj].ToggleOtr_->setChecked (true);
		}
	}

	void OtrHandler::HandleEntryActionsRemoved (QObject *entry)
	{
		Entry2Action_.remove (entry);
	}

	void OtrHandler::HandleEntryActionsRequested (const IHookProxy_ptr& proxy, QObject *entry)
	{
		if (qobject_cast<ICLEntry*> (entry)->GetEntryType () == ICLEntry::EntryType::MUC)
			return;

		if (!Entry2Action_.contains (entry))
			CreateActions (entry);

		auto list = proxy->GetReturnValue ().toList ();

		const auto& actionsStruct = Entry2Action_.value (entry);
		const auto actions =
		{
			actionsStruct.ToggleOtr_.get (),
			actionsStruct.ToggleOtrCtx_.get (),
			actionsStruct.CtxMenu_->menuAction ()
		};
		for (const auto action : actions)
			list << QVariant::fromValue<QObject*> (action);

		proxy->SetReturnValue (list);
	}

	void OtrHandler::HandleEntryActionAreasRequested (const IHookProxy_ptr& proxy, QObject *action)
	{
		if (!action->property ("Azoth/OTRoid/IsGood").toBool ())
			return;

		const auto& ours = action->property ("Azoth/OTRoid/Areas").toStringList ();
		proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
	}

	OtrlUserState OtrHandler::GetUserState () const
	{
		return UserState_;
	}

	int OtrHandler::IsLoggedIn (const QString& accId, const QString& entryId)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		if (!entry)
			return -1;

		return entry->Variants ().isEmpty () ? 0 : 1;
	}

	void OtrHandler::InjectMsg (const QString& accId, const QString& entryId,
			const QString& body, bool hidden, IMessage::Direction dir, IMessage::Type type)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "no such entry"
					<< entryId
					<< accId;
			return;
		}

		InjectMsg (entry, body, hidden, dir, type);
	}

	void OtrHandler::InjectMsg (ICLEntry *entry, const QString& body, bool hidden,
			IMessage::Direction dir, IMessage::Type type)
	{
		if (dir == IMessage::Direction::Out)
		{
			const auto msg = entry->CreateMessage (type, {}, body);
			if (hidden)
				msg->GetQObject ()->setProperty ("Azoth/HiddenMessage", true);

			msg->ToggleOTRMessage (true);
			msg->Send ();
		}
		else
		{
			auto entryObj = entry->GetQObject ();
			auto msgObj = AzothProxy_->CreateCoreMessage (body,
					QDateTime::currentDateTime (),
					type, dir, entryObj, entryObj);

			PendingInjectedMessages_ << msgObj;

			auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();
		}
	}

	void OtrHandler::Notify (const QString&, const QString&,
			Priority prio, const QString& title,
			const QString& prim, const QString& sec)
	{
		QString text = prim;
		if (!sec.isEmpty ())
			text += "<br />" + sec;

		CoreProxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification (title, text, prio));
	}

	QString OtrHandler::GetAccountName (const QString& accId)
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

	QString OtrHandler::GetVisibleEntryName (const QString& accId, const QString& entryId)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "no such entry"
					<< entryId
					<< accId;
			return entryId;
		}

		return GetVisibleEntryNameImpl (entry);
	}

	void OtrHandler::CreatePrivkey (const char *accName, const char *proto, bool confirm)
	{
		if (IsGenerating_)
			return;

		const auto& hrAccName = GetAccountName (QString::fromUtf8 (accName));
		if (confirm && QMessageBox::question (nullptr,
				"Azoth OTRoid",
				tr ("Private keys for account %1 need to be generated. This takes quite some "
					"time (from a few seconds to a couple of minutes), and while you can use "
					"LeechCraft in the meantime, all the messages will be sent unencrypted "
					"until keys are generated. You will be notified when this process finishes. "
					"Do you want to generate keys now?"
					"<br /><br />You can also move mouse randomily to help generating entropy.")
					.arg (hrAccName),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const auto& notify = Util::MakeNotification ("Azoth OTRoid",
				tr ("Keys for account %1 are now being generated...")
					.arg ("<em>" + hrAccName + "</em>"),
				Priority::Info);
		CoreProxy_->GetEntityManager ()->HandleEntity (notify);

		IsGenerating_ = true;

		const auto keysFile = GetOTRFilename ("privkey");

		QEventLoop loop;
		QFutureWatcher<gcry_error_t> watcher;
		connect (&watcher,
				SIGNAL (finished ()),
				&loop,
				SLOT (quit ()));
		auto future = QtConcurrent::run (otrl_privkey_generate,
				UserState_, keysFile.constData (), accName, proto);
		watcher.setFuture (future);

		loop.exec ();

		IsGenerating_ = false;

		char fingerprint [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		if (!otrl_privkey_fingerprint (UserState_, fingerprint, accName, proto))
		{
			qWarning () << Q_FUNC_INFO
					<< "failed";
			return;
		}

		emit privKeysChanged ();

		QMessageBox::information (nullptr,
				"Azoth OTRoid",
				tr ("Keys are generated. Thanks for your patience."));
	}

	void OtrHandler::CreateInstag (const char *accName, const char *proto)
	{
		otrl_instag_generate (UserState_,
				GetOTRFilename ("instags").constData (), accName, proto);
	}

	void OtrHandler::SetPollTimerInterval (unsigned int seconds)
	{
		if (PollTimer_->isActive ())
			PollTimer_->stop ();

		if (seconds)
			PollTimer_->start (seconds * 1000);
	}

	void OtrHandler::HandleSmpEvent (OtrlSMPEvent smpEvent,
			ConnContext *context, unsigned short progressPercent, const QString& question)
	{
		qDebug () << Q_FUNC_INFO << smpEvent << progressPercent << question;

		const auto entryObj = AzothProxy_->GetEntry (context->username, context->accountname);
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "no such entry"
					<< context->username
					<< context->accountname;
			return;
		}

		if (!Auths_.contains (entry))
			CreateAuthForEntry (entry);

		const auto auth = Auths_.value (entry);

		switch (smpEvent)
		{
		case OTRL_SMPEVENT_ASK_FOR_SECRET:
			auth->AskFor (SmpMethod::SharedSecret, question, context);
			break;
		case OTRL_SMPEVENT_ASK_FOR_ANSWER:
			auth->AskFor (SmpMethod::Question, question, context);
			break;
		case OTRL_SMPEVENT_ERROR:
		case OTRL_SMPEVENT_ABORT:
		case OTRL_SMPEVENT_FAILURE:
			auth->Failed ();
			break;
		case OTRL_SMPEVENT_CHEATED:
			auth->Cheated ();
			break;
		case OTRL_SMPEVENT_SUCCESS:
			auth->Success ();
			break;
		case OTRL_SMPEVENT_IN_PROGRESS:
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown SMP event";
			break;
		}
	}

	void OtrHandler::CreateAuthForEntry (ICLEntry *entry)
	{
		const auto auth = new Authenticator { entry };
		connect (auth,
				SIGNAL (abortSmp (ConnContext*)),
				this,
				SLOT (handleAbortSmp (ConnContext*)));
		connect (auth,
				SIGNAL (gotReply (SmpMethod, QString, ConnContext*)),
				this,
				SLOT (handleGotSmpReply (SmpMethod, QString, ConnContext*)));
		connect (auth,
				SIGNAL (initiateRequested (ICLEntry*, SmpMethod, QString, QString)),
				this,
				SLOT (startAuth (ICLEntry*, SmpMethod, QString, QString)));
		connect (auth,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleAuthDestroyed ()));
		Auths_ [entry] = auth;
	}

	void OtrHandler::writeFingerprints ()
	{
		otrl_privkey_write_fingerprints (UserState_,
				GetOTRFilename ("fingerprints").constData ());
	}

	void OtrHandler::writeKeys ()
	{
		WriteKeys (UserState_, GetOTRFilename ("privkey"));
	}

	void OtrHandler::generateKeys (const QString& acc, const QString& proto)
	{
		CreatePrivkey (acc.toUtf8 ().constData (), proto.toUtf8 ().constData (), false);
	}

	void OtrHandler::handleOtrAction ()
	{
		auto act = qobject_cast<QAction*> (sender ());
		auto entryObj = act->property ("Azoth/OTRoid/Entry").value<QObject*> ();
		SetOtrState (qobject_cast<ICLEntry*> (entryObj), act->isChecked ());
	}

	QByteArray OtrHandler::GetOTRFilename (const QString& fname) const
	{
		return OtrDir_.absoluteFilePath (fname).toUtf8 ();
	}

	void OtrHandler::CreateActions (QObject *entry)
	{
		auto makeOtrAction = [entry, this] () -> std::shared_ptr<QAction>
		{
			const auto& otr = std::make_shared<QAction> (tr ("Enable OTR"), this);
			otr->setCheckable (true);
			otr->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());
			otr->setProperty ("Azoth/OTRoid/IsGood", true);
			otr->setProperty ("Azoth/OTRoid/Entry", QVariant::fromValue (entry));
			return otr;
		};

		const auto& otr = makeOtrAction ();
		otr->setProperty ("Azoth/OTRoid/Areas", QStringList { "tabContextMenu", "toolbar" });
		connect (otr.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (handleOtrAction ()));

		const auto& otrCtx = makeOtrAction ();
		otrCtx->setProperty ("Azoth/OTRoid/Areas", QStringList { "contactListContextMenu" });
		connect (otrCtx.get (),
				SIGNAL (toggled (bool)),
				otr.get (),
				SLOT (setChecked (bool)));
		connect (otrCtx.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (handleOtrAction ()));
		connect (otr.get (),
				SIGNAL (toggled (bool)),
				otrCtx.get (),
				SLOT (setChecked (bool)));

		const auto& buttonMenu = std::make_shared<QMenu> ();
		otr->setMenu (buttonMenu.get ());

		const auto& ctxMenu = std::make_shared<QMenu> (tr ("OTR"));
		ctxMenu->setProperty ("Azoth/OTRoid/IsGood", true);
		ctxMenu->setProperty ("Azoth/OTRoid/Areas", QStringList { "contactListContextMenu" });

		const auto& auth = std::make_shared<QAction> (tr ("Authenticate the contact"), this);
		auth->setProperty ("Azoth/OTRoid/IsGood", true);
		auth->setProperty ("Azoth/OTRoid/Areas", QStringList { "contactListContextMenu" });
		auth->setProperty ("Azoth/OTRoid/Entry", QVariant::fromValue (entry));
		connect (auth.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (handleAuthRequested ()));

		buttonMenu->addAction (auth.get ());
		ctxMenu->addAction (auth.get ());

		Entry2Action_ [entry] = EntryActions
		{
			ctxMenu,
			buttonMenu,
			otr,
			otrCtx,
			auth
		};
	}

	void OtrHandler::SetOtrState (ICLEntry *entry, bool enable)
	{
		const auto acc = entry->GetParentAccount ();
		const auto& accId = acc->GetAccountID ();

		auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
		const auto& protoId = proto->GetProtocolID ();

		if (!enable)
		{
			otrl_message_disconnect (UserState_, &OtrOps_, this,
					accId.constData (), protoId.constData (),
					entry->GetEntryID ().toUtf8 ().constData (), OTRL_INSTAG_BEST);
			const auto& message = tr ("Private conversation closed");
			InjectMsg (acc->GetAccountID (), entry->GetEntryID (),
						message, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
			return;
		}
		else
		{
			const auto& message = tr ("Attempting to start a private conversation");
			InjectMsg (acc->GetAccountID (), entry->GetEntryID (),
					   message, false, IMessage::Direction::In, IMessage::Type::ServiceMessage);
		}

		char fingerprint [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		if (!otrl_privkey_fingerprint (UserState_, fingerprint,
				accId.constData (), protoId.constData ()))
			CreatePrivkey (accId.constData (), protoId.constData());

		std::shared_ptr<char> msg (otrl_proto_default_query_msg (accId.constData (),
					OTRL_POLICY_DEFAULT), free);
		InjectMsg (entry, QString::fromUtf8 (msg.get ()), true, IMessage::Direction::Out);
	}

	void OtrHandler::handleAuthRequested ()
	{
		auto act = qobject_cast<QAction*> (sender ());

		auto entryObj = act->property ("Azoth/OTRoid/Entry").value<QObject*> ();
		auto entry = qobject_cast<ICLEntry*> (entryObj);

		if (!Entry2Action_ [entryObj].ToggleOtr_->isChecked ())
		{
			if (QMessageBox::question (nullptr,
						"LeechCraft",
						tr ("You need to start a private conversation before authentication "
							"can take place. Do you want to start it?"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				return;

			SetOtrState (entry, true);
		}

		if (!Auths_.contains (entry))
			CreateAuthForEntry (entry);

		const auto auth = Auths_.value (entry);
		auth->Initiate ();
	}

	void OtrHandler::startAuth (ICLEntry *entry, SmpMethod method,
			const QString& questionStr, const QString& answerStr)
	{
		const auto acc = entry->GetParentAccount ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		const auto context = otrl_context_find (UserState_,
				entry->GetEntryID ().toUtf8 ().constData (),
				acc->GetAccountID ().constData (),
				proto->GetProtocolID ().constData (),
				OTRL_INSTAG_BEST,
				true,
				nullptr, nullptr, nullptr);

		const auto& question = questionStr.toUtf8 ();
		const auto& answer = answerStr.toUtf8 ();
		switch (method)
		{
		case SmpMethod::Question:
			otrl_message_initiate_smp_q (UserState_, &OtrOps_, this,
					context,
					question.constData (),
					reinterpret_cast<const unsigned char*> (answer.constData ()), answer.size ());
			break;
		case SmpMethod::SharedSecret:
			otrl_message_initiate_smp (UserState_, &OtrOps_, this,
					context,
					reinterpret_cast<const unsigned char*> (answer.constData ()), answer.size ());
			break;
		}
	}

	void OtrHandler::handleAuthDestroyed ()
	{
		const auto auth = static_cast<Authenticator*> (sender ());
		const auto key = Auths_.key (auth);
		Auths_.remove (key);
	}

	void OtrHandler::handleGotSmpReply (SmpMethod, const QString& reply, ConnContext *context)
	{
		const auto& replyUtf = reply.toUtf8 ();
		otrl_message_respond_smp (UserState_, &OtrOps_, this, context,
				reinterpret_cast<const unsigned char*> (replyUtf.constData ()), replyUtf.size ());
	}

	void OtrHandler::handleAbortSmp (ConnContext *context)
	{
		otrl_message_abort_smp (UserState_, &OtrOps_, this, context);
	}

	void OtrHandler::pollOTR ()
	{
		otrl_message_poll (UserState_, &OtrOps_, this);
	}
}
}
}
