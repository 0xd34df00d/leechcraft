/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "otrhandler.h"
#include <cstdio>
#include <cstring>
#include <utility>
#include <gcrypt.h>
#include <unistd.h>
#include <util/gui/asyncdialogs.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/util.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include <QMenu>
#include <QMessageBox>
#include <QFile>
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
#include <util/azoth/entryidops.h>
#include <util/azoth/util.h>
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
		constexpr int UnknownUser = -1;

		int IsLoggedIn (void *opData, const char *accName,
				const char*, const char *recipient)
		{
			const auto entryId = Deserialize<EntryStrongestId> (QString::fromUtf8 (recipient));
			return entryId ?
					static_cast<OtrHandler*> (opData)->IsLoggedIn (AccountId::FromUtf8 (accName), *entryId) :
					UnknownUser;
		}

		void InjectMessage (void *opData, const char *accName,
				const char*, const char *recipient, const char *msg)
		{
			const auto plugin = static_cast<OtrHandler*> (opData);
			const auto entryId = Deserialize<EntryStrongestId> (QString::fromUtf8 (recipient));
			if (!entryId)
				return;

			if (const auto entry = plugin->GetEntry (AccountId::FromUtf8 (accName), *entryId))
				entry->SendMessage ({
							.Body_ = QString::fromUtf8 (msg),
							.AllowServerArchival_ = false,
							.Hidden_ = true,
						});
			else
				qWarning () << "unknown entry" << accName << recipient;
		}

		void Notify (void *opData, OtrlNotifyLevel level,
				const char*, const char*,
				const char*, const char *title,
				const char *primary, const char *secondary)
		{
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

			auto u = [] (const char *cs) { return QString::fromUtf8 (cs); };
			static_cast<OtrHandler*> (opData)->Notify (prio, u (title), u (primary), u (secondary));
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
			qDebug () << event << message;

			auto plugin = static_cast<OtrHandler*> (opData);
			const auto entryId = Deserialize<EntryStrongestId> (QString::fromUtf8 (context->username));
			if (!entryId)
				return;

			const auto& contact = plugin->GetVisibleEntryName (AccountId::FromUtf8 (context->accountname), *entryId);

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
					msg += ' ' + OtrHandler::tr ("Original OTR message: %1.").arg (QString::fromUtf8 (message));
				plugin->NotifyConvFlow (context, msg);
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
			const auto hrHash = QString::fromLatin1 (fpHash); // human readable fingerprint

			const auto plugin = static_cast<OtrHandler*> (opData);
			const auto entryId = Deserialize<EntryStrongestId> (QString::fromUtf8 (username));
			if (!entryId)
				return;

			const auto& accountId = AccountId::FromUtf8 (accountname);
			const auto& msg = OtrHandler::tr ("You have received a new fingerprint from %1: %2")
					.arg (plugin->GetVisibleEntryName (accountId, *entryId), hrHash);
			plugin->NotifyConvFlow (accountId, *entryId, msg);
		}

		void HandleGoneSecure (void *opData, ConnContext *ctx)
		{
			static_cast<OtrHandler*> (opData)->NotifyConvFlow (ctx, OtrHandler::tr ("Private conversation started"));
		}

		void HandleGoneInsecure (void *opData, ConnContext *ctx)
		{
			static_cast<OtrHandler*> (opData)->NotifyConvFlow (ctx, OtrHandler::tr ("Private conversation lost"));
		}

		void HandleStillSecure (void *opData, ConnContext *ctx, int)
		{
			static_cast<OtrHandler*> (opData)->NotifyConvFlow (ctx, QObject::tr ("Private conversation refreshed"));
		}
	}

	OtrHandler::OtrHandler (const ICoreProxy_ptr& coreProxy, IProxyObject *azothProxy)
	: CoreProxy_ { coreProxy }
	, AzothProxy_ { azothProxy }
	, OtrDir_ { Util::CreateIfNotExists ("azoth/otr/") }
	, UserState_ { otrl_userstate_create () }
	, FragmentsState_ { otrl_userstate_create () }
	{
		otrl_privkey_read (UserState_, GetOTRFilename ("privkey").constData ());
		otrl_privkey_read_fingerprints (UserState_,
				GetOTRFilename ("fingerprints").constData (), NULL, NULL);
		otrl_instag_read (UserState_, GetOTRFilename ("instags").constData ());

		memset (&OtrOps_, 0, sizeof (OtrOps_));
		OtrOps_.policy = [] (void*, ConnContext*) { return OtrlPolicy { OTRL_POLICY_DEFAULT }; };
		OtrOps_.create_privkey = [] (void *opData, const char *accName, const char*)
				{ static_cast<OtrHandler*> (opData)->PrivkeysRequested_ << AccountId::FromUtf8 (accName); };
		OtrOps_.create_instag = [] (void *opData, const char *accName, const char *proto)
				{ static_cast<OtrHandler*> (opData)->CreateInstag (accName, proto); };
		OtrOps_.is_logged_in = &OTR::IsLoggedIn;
		OtrOps_.inject_message = &OTR::InjectMessage;
		OtrOps_.update_context_list = [] (void*) {};
		OtrOps_.new_fingerprint = &OTR::HandleNewFingerprint;
		OtrOps_.write_fingerprints = [] (void *opData) { static_cast<OtrHandler*> (opData)->writeFingerprints (); };
		OtrOps_.account_name = [] (void *opData, const char *acc, const char*) -> const char*
				{
					const auto& name = static_cast<OtrHandler*> (opData)->
							GetAccountName (AccountId::FromUtf8 (acc)).toUtf8 ();

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
				&QTimer::timeout,
				this,
				[this] { otrl_message_poll (UserState_, &OtrOps_, this); });

		SetPollTimerInterval (otrl_message_poll_get_default_interval (UserState_));
	}

	OtrHandler::~OtrHandler ()
	{
		otrl_userstate_free (UserState_);
		otrl_userstate_free (FragmentsState_);
	}

	void OtrHandler::HandleMessageCreated (bool& cancel, ICLEntry& entry, OutgoingMessage& msg)
	{
		const auto entryObj = entry.GetQObject ();
		if (!Entry2Action_.contains (entryObj) ||
				!Entry2Action_ [entryObj].ToggleOtr_->isChecked ())
			return;

		const auto acc = entry.GetParentAccount ();
		const auto accName = acc->GetAccountId ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		char *newMsg = 0;
		const auto err = otrl_message_sending (UserState_,
				&OtrOps_,
				this,
				accName.ToUtf8 ().constData (),
				proto->GetProtocolId ().toUtf8 ().constData (),
				SerializeString (entry.GetStrongestId ()).toUtf8 ().constData (),
				OTRL_INSTAG_BEST,
				msg.Body_.toUtf8 ().constData (),
				nullptr,
				&newMsg,
				OTRL_FRAGMENT_SEND_SKIP,
				nullptr,
				nullptr,
				nullptr);

		if (err)
		{
			qWarning () << "OTR error occured, aborting";
			cancel = true;
			return;
		}

		if (newMsg)
		{
			AzothProxy_->InjectMessage (entry, InjectedMessage::FromOutgoing (msg));

			msg.Body_ = QString::fromUtf8 (newMsg);
			msg.RichTextBody_.reset ();
			msg.AllowServerArchival_ = false;
			msg.Hidden_ = true;
		}

		otrl_message_free (newMsg);
	}

	OtrHandler::ReceivingResult_t OtrHandler::RunOtrReceiving (ICLEntry& entry, const QString& body)
	{
		char *newMsg = nullptr;
		const auto newMsgGuard = Util::MakeScopeGuard ([&] { if (newMsg) otrl_message_free (newMsg); });

		const auto acc = entry.GetParentAccount ();
		const auto accName = acc->GetAccountId ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		OtrlTLV *tlvs = nullptr;
		const auto tlvsGuard = Util::MakeScopeGuard ([&] { otrl_tlv_free (tlvs); });
		auto ignore = otrl_message_receiving (UserState_, &OtrOps_, this,
				accName.ToUtf8 ().constData (),
				proto->GetProtocolId ().toUtf8 ().constData (),
				SerializeString (entry.GetStrongestId ()).toUtf8 ().constData (),
				body.toUtf8 ().constData (),
				&newMsg,
				&tlvs,
				nullptr,
				nullptr,
				nullptr);

		if (otrl_tlv_find (tlvs, OTRL_TLV_DISCONNECTED))
		{
			const auto& message = tr ("%1 has ended the private conversation with you, you should do the same.")
					.arg (FormatNameWithAddress (entry));
			NotifyConvFlow (entry, message);
			const auto& plaintext = newMsg ? std::optional { QString::fromUtf8 (newMsg) } : std::nullopt;
			return ReceivingOtrEnd { plaintext };
		}

		// Magic hack to force it work similar to libotr < 4.0.0.
		// If user received unencrypted message they should be notified.
		// See OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED as well.
		if (!body.startsWith("?OTR") && ignore && !newMsg)
			ignore = 0;

		if (ignore)
			return ReceivingIgnore {};

		if (newMsg)
			return ReceivingDecrypted { QString::fromUtf8 (newMsg) };

		return ReceivingNonOtr {};
	}

	// libotr keeps no copy of a handshake message it drops for the lack of a private key,
	// and feeding it one again takes the whole message, not the last fragment the hook gets.
	// The context is chosen the way the fragment handling at the start of otrl_message_receiving () does it.
	std::optional<QString> OtrHandler::Reassemble (ICLEntry& entry, const QString& body)
	{
		const auto& bodyUtf8 = body.toUtf8 ();
		const auto otrPos = bodyUtf8.indexOf ("?OTR");
		if (otrPos < 0)
			return body;

		const auto acc = entry.GetParentAccount ();
		const auto& accId = acc->GetAccountId ().ToUtf8 ();
		const auto& protoId = qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolId ().toUtf8 ();
		if (otrl_privkey_find (UserState_, accId.constData (), protoId.constData ()))
			return {};

		const auto& entryId = SerializeString (entry.GetStrongestId ()).toUtf8 ();
		const auto findContext = [&] (otrl_instag_t tag)
		{
			return otrl_context_find (FragmentsState_, entryId.constData (), accId.constData (), protoId.constData (),
					tag, true, nullptr, nullptr, nullptr);
		};
		// otrl_context_find () takes a contact's first context for the master, so it has to exist before an instance's.
		auto context = findContext (OTRL_INSTAG_MASTER);

		if (QByteArrayView { bodyUtf8 }.sliced (otrPos).startsWith ("?OTR|"))
		{
			otrl_instag_t senderTag = 0;
			otrl_instag_t recipientTag = 0;
			std::sscanf (bodyUtf8.constData () + otrPos, "?OTR|%x|%x,", &senderTag, &recipientTag);

			const auto ourInstag = otrl_instag_find (UserState_, accId.constData (), protoId.constData ());
			const auto isForUs = !recipientTag || (ourInstag && ourInstag->instag == recipientTag);
			// otrl_context_find () returns nullptr for the reserved instance tags.
			if (!isForUs || senderTag < OTRL_MIN_VALID_INSTAG)
				return {};

			context = findContext (senderTag);
		}

		char *whole = nullptr;
		const auto wholeGuard = Util::MakeScopeGuard ([&] { free (whole); });
		switch (otrl_proto_fragment_accumulate (&whole, context, bodyUtf8.constData ()))
		{
		case OTRL_FRAGMENT_UNFRAGMENTED:
			return body;
		case OTRL_FRAGMENT_INCOMPLETE:
			return {};
		case OTRL_FRAGMENT_COMPLETE:
			return QString::fromUtf8 (whole);
		}
		std::unreachable ();
	}

	void OtrHandler::HandleGotMessage (const IHookProxy_ptr& proxy, QObject *msgObj)
	{
		auto& msg = qobject_ref_cast<IMessage> (msgObj);

		if (msg.IsForwarded ())
		{
			if (msg.GetBody ().startsWith ("?OTR"))
			{
				proxy->CancelDefault ();
				msgObj->setProperty ("Azoth/HiddenMessage", true);
			}
			return;
		}

		if (msg.GetMessageType () != IMessage::Type::ChatMessage ||
			msg.GetDirection () != IMessage::Direction::In)
			return;

		const auto entryObj = msg.ParentCLEntry ();
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry ||
				entry->GetEntryType () == ICLEntry::EntryType::MUC)
			return;

		const auto accName = entry->GetParentAccount ()->GetAccountId ();

		const auto& wholeBody = Reassemble (*entry, msg.GetBody ());

		const auto isOtrMessage = Util::Visit (RunOtrReceiving (*entry, msg.GetBody ()),
				[&] (const ReceivingIgnore&)
				{
					proxy->CancelDefault ();
					msgObj->setProperty ("Azoth/HiddenMessage", true);
					return true;
				},
				[&] (const ReceivingNonOtr&) { return false; },
				[&] (const ReceivingOtrEnd& end)
				{
					if (end.PlaintextMsg_)
					{
						msg.SetBody (*end.PlaintextMsg_);
					}
					else
					{
						proxy->CancelDefault ();
						msgObj->setProperty ("Azoth/HiddenMessage", true);
					}
					return false;
				},
				[&] (const ReceivingDecrypted& decrypted)
				{
					msg.SetBody (decrypted.Body_);
					return true;
				});

		msg.ToggleOTRMessage (isOtrMessage);
		if (!Entry2Action_.contains (entryObj))
			CreateActions (entryObj);
		if (isOtrMessage)
			SetOtrActionsState (*entry, true);

		if (PrivkeysRequested_.remove (accName))
			ResumeWithPrivkey (entry, wholeBody);
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

	int OtrHandler::IsLoggedIn (const AccountId& accId, const EntryStrongestId& entryId)
	{
		if (const auto entry = GetEntry (accId, entryId))
			return entry->Variants ().isEmpty () ? 0 : 1;
		return OTR::UnknownUser;
	}

	void OtrHandler::NotifyConvFlow (const ConnContext *ctx, const QString& msg)
	{
		if (const auto entryId = Deserialize<EntryStrongestId> (QString::fromUtf8 (ctx->username)))
			NotifyConvFlow (AccountId::FromUtf8 (ctx->accountname), *entryId, msg);
	}

	void OtrHandler::NotifyConvFlow (const AccountId& accId, const EntryStrongestId& entryId, const QString& msg)
	{
		if (const auto entry = GetEntry (accId, entryId))
			NotifyConvFlow (*entry, msg);
		else
			qWarning () << "no such entry" << entryId << accId;
	}

	void OtrHandler::NotifyConvFlow (ICLEntry& entry, const QString& msg)
	{
		AzothProxy_->InjectMessage (entry, { .Body_ = msg, .Kind_ = InjectedMessage::Service {} });
	}

	void OtrHandler::Notify (Priority prio, const QString& title, const QString& prim, const QString& sec)
	{
		QString text = prim;
		if (!sec.isEmpty ())
			text += "<br />" + sec;

		CoreProxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification (title, text, prio));
	}

	QString OtrHandler::GetAccountName (const AccountId& accId)
	{
		QObject *accObj = AzothProxy_->GetAccount (accId.ToString ());
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

	ICLEntry* OtrHandler::GetEntry (const AccountId& accId, const EntryStrongestId& entryId)
	{
		return AzothProxy_->GetEntry (AsGlobalId (accId, entryId));
	}

	QString OtrHandler::GetVisibleEntryName (const AccountId& accId, const EntryStrongestId& entryId)
	{
		const auto entry = GetEntry (accId, entryId);
		if (!entry)
		{
			qWarning () << "no such entry" << entryId << accId;
			return entryId.ToString ();
		}

		return FormatNameWithAddress (*entry);
	}

	Util::SharedContextTask<OtrHandler::KeyGenResult> OtrHandler::CreatePrivkey (const AccountId& accName, const QString& proto, bool confirm)
	{
		auto pos = PrivkeyCreationTasks_.find (accName);
		if (pos == PrivkeyCreationTasks_.end ())
			pos = PrivkeyCreationTasks_.emplace (accName, RunCreatePrivkey (accName, proto, confirm));
		return *pos;
	}

	namespace
	{
		gcry_error_t FinishKeyGeneration (OtrlUserState state, void *newKeyCtx, const QString& keysPath)
		{
			QFile file { keysPath };
			constexpr auto perms = QFile::ReadOwner | QFile::WriteOwner;
			const auto keysCFile = file.open (QFile::ReadWrite | QFile::Truncate, perms) ?
					fdopen (dup (file.handle ()), "w+b") :
					nullptr;
			if (!keysCFile)
			{
				const auto openErr = gcry_error_from_errno (errno);
				otrl_privkey_generate_cancelled (state, newKeyCtx);
				return openErr ? openErr : gcry_error (GPG_ERR_GENERAL);
			}

			const auto err = otrl_privkey_generate_finish_FILEp (state, newKeyCtx, keysCFile);
			fclose (keysCFile);
			return err;
		}
	}

	Util::SharedContextTask<OtrHandler::KeyGenResult> OtrHandler::RunCreatePrivkey (AccountId accName, QString proto, bool confirm)
	{
		co_await Util::AddContext { CoroContext_ };

		const auto& hrAccName = GetAccountName (accName);

		using namespace std::chrono_literals;
		// ensure the task is stored in `PrivkeyCreationTasks_`
		co_await 0ms;

		const auto guard = Util::MakeScopeGuard ([this, accName] { PrivkeyCreationTasks_.remove (accName); });

		if (confirm && co_await Util::Dialogs::Question (nullptr,
				"Azoth OTRoid",
				tr ("Private keys for account %1 need to be generated. This takes some "
					"time (up to a couple of minutes), although you can use "
					"LeechCraft in the meantime. You will be notified when this process finishes. "
					"Do you want to generate the keys now?")
					.arg (hrAccName),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			co_return KeyGenResult::Rejected;

		const auto& notify = Util::MakeNotification ("Azoth OTRoid",
				tr ("Keys for account %1 are now being generated...")
					.arg ("<em>" + hrAccName + "</em>"),
				Priority::Info);
		CoreProxy_->GetEntityManager ()->HandleEntity (notify);

		auto report = [hrAccName] (this auto, gcry_error_t err)
		{
			qWarning () << "gcry error" << err;
			Util::Dialogs::Warning (nullptr,
					"Azoth OTRoid",
					tr ("Keys generation for account %1 failed: %2.").arg (hrAccName, QString::fromUtf8 (gcry_strerror (err))));
		};

		void *newKeyCtx;
		if (const auto err = otrl_privkey_generate_start (UserState_, accName.ToUtf8 ().constData (), proto.toUtf8 ().constData (), &newKeyCtx))
		{
			report (err);
			co_return KeyGenResult::Failed;
		}

		if (const auto calcError = co_await QtConcurrent::run (&otrl_privkey_generate_calculate, newKeyCtx))
		{
			otrl_privkey_generate_cancelled (UserState_, newKeyCtx);
			report (calcError);
			co_return KeyGenResult::Failed;
		}

		if (const auto err = FinishKeyGeneration (UserState_, newKeyCtx, OtrDir_.absoluteFilePath ("privkey")))
		{
			report (err);
			co_return KeyGenResult::Failed;
		}

		emit privKeysChanged ();

		Util::Dialogs::Information (nullptr,
				"Azoth OTRoid",
				tr ("Keys are generated. Thanks for your patience."));

		co_return KeyGenResult::Created;
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

		const auto entryId = Deserialize<EntryStrongestId> (QString::fromUtf8 (context->username));
		if (!entryId)
			return;

		const auto entry = GetEntry (AccountId::FromUtf8 (context->accountname), *entryId);
		if (!entry)
		{
			qWarning () << "no such entry" << entryId << context->accountname;
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
				&Authenticator::abortSmp,
				this,
				[this] (ConnContext *context) { otrl_message_abort_smp (UserState_, &OtrOps_, this, context); });
		connect (auth,
				&Authenticator::gotReply,
				this,
				[this] (SmpMethod, const QString& reply, ConnContext *context)
				{
					const auto& replyBytes = reply.toUtf8 ();
					otrl_message_respond_smp (UserState_, &OtrOps_, this, context,
							reinterpret_cast<const unsigned char*> (replyBytes.constData ()), replyBytes.size ());
				});
		connect (auth,
				&Authenticator::initiateRequested,
				this,
				&OtrHandler::StartAuth);
		connect (auth,
				&Authenticator::destroyed,
				this,
				[this, entry] { Auths_.remove (entry); });
		Auths_ [entry] = auth;
	}

	void OtrHandler::writeFingerprints ()
	{
		otrl_privkey_write_fingerprints (UserState_,
				GetOTRFilename ("fingerprints").constData ());
	}

	void OtrHandler::writeKeys ()
	{
		WriteKeys (UserState_, OtrDir_.absoluteFilePath ("privkey"));
	}

	void OtrHandler::generateKeys (const AccountId& acc, const QString& proto)
	{
		CreatePrivkey (acc, proto, false);
	}

	QByteArray OtrHandler::GetOTRFilename (const QString& fname) const
	{
		return QFile::encodeName (OtrDir_.absoluteFilePath (fname));
	}

	void OtrHandler::CreateActions (QObject *entry)
	{
		auto makeOtrAction = [entry, this]
		{
			const auto& otr = std::make_shared<QAction> (tr ("Enable OTR"), this);
			otr->setCheckable (true);
			otr->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());
			otr->setProperty ("Azoth/OTRoid/IsGood", true);
			otr->setProperty ("Azoth/OTRoid/Entry", QVariant::fromValue (entry));
			return otr;
		};

		auto handleOtrAction = [entry, this] (bool checked) { SetOtrState (qobject_cast<ICLEntry*> (entry), checked); };

		const auto& otr = makeOtrAction ();
		otr->setProperty ("Azoth/OTRoid/Areas", QStringList { "tabContextMenu", "toolbar" });
		connect (otr.get (),
				&QAction::triggered,
				this,
				handleOtrAction);

		const auto& otrCtx = makeOtrAction ();
		otrCtx->setProperty ("Azoth/OTRoid/Areas", QStringList { "contactListContextMenu" });
		connect (otrCtx.get (),
				&QAction::toggled,
				otr.get (),
				&QAction::setChecked);
		connect (otrCtx.get (),
				&QAction::triggered,
				this,
				handleOtrAction);
		connect (otr.get (),
				&QAction::toggled,
				otrCtx.get (),
				&QAction::setChecked);

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
				&QAction::triggered,
				this,
				[this, entry] { HandleAuthRequested (qobject_cast<ICLEntry*> (entry)); });

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

	Util::ContextTask<OtrHandler::OtrState> OtrHandler::SetOtrState (Util::TrackingPointer<ICLEntry> entry, bool enable)
	{
		co_await Util::AddContext { CoroContext_ };

		const auto acc = entry->GetParentAccount ();
		const auto& accId = acc->GetAccountId ();
		const auto& accIdUtf8 = accId.ToUtf8 ();

		const auto& protoId = qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolId ();
		const auto& protoIdUtf8 = protoId.toUtf8 ();

		if (!enable)
		{
			otrl_message_disconnect (UserState_, &OtrOps_, this,
					accIdUtf8.constData (), protoIdUtf8.constData (),
					SerializeString (entry->GetStrongestId ()).toUtf8 ().constData (), OTRL_INSTAG_BEST);
			NotifyConvFlow (*entry, tr ("Private conversation closed"));
			co_return OtrState::Disabled;
		}

		NotifyConvFlow (*entry, tr ("Attempting to start a private conversation"));

		char fingerprint [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		if (!otrl_privkey_fingerprint (UserState_, fingerprint,
				accIdUtf8.constData (), protoIdUtf8.constData ()))
		{
			const auto result = co_await CreatePrivkey (accId, protoId);
			const bool enabled = result == KeyGenResult::Created;
			switch (result)
			{
			case KeyGenResult::Created:
				break;
			case KeyGenResult::Rejected:
				NotifyConvFlow (*entry, tr ("Key generation rejected, private conversation not started"));
				break;
			case KeyGenResult::Failed:
				NotifyConvFlow (*entry, tr ("Key generation failed, private conversation not started"));
				break;
			}
			SetOtrActionsState (*entry, enabled);
			if (!enabled)
				co_return OtrState::Disabled;
		}

		SendOtrQuery (*entry);
		co_return OtrState::QuerySent;
	}

	void OtrHandler::SendOtrQuery (ICLEntry& entry)
	{
		const auto& accId = entry.GetParentAccount ()->GetAccountId ().ToUtf8 ();
		const std::shared_ptr<char> msg { otrl_proto_default_query_msg (accId.constData (), OTRL_POLICY_DEFAULT), free };
		entry.SendMessage ({ .Body_ = QString::fromUtf8 (msg.get ()), .Hidden_ = true });
	}

	Util::ContextTask<void> OtrHandler::ResumeWithPrivkey (Util::TrackingPointer<ICLEntry> entry, std::optional<QString> body)
	{
		co_await Util::AddContext { CoroContext_ };

		const auto acc = entry->GetParentAccount ();
		const auto accId = acc->GetAccountId ();
		const auto& protoId = qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolId ();

		if (co_await CreatePrivkey (accId, protoId) != KeyGenResult::Created)
		{
			SetOtrActionsState (*entry, false);
			co_return;
		}

		const auto findContext = [&] (otrl_instag_t instance)
		{
			return otrl_context_find (UserState_,
					SerializeString (entry->GetStrongestId ()).toUtf8 ().constData (),
					accId.ToUtf8 ().constData (), protoId.toUtf8 ().constData (),
					instance, false, nullptr, nullptr, nullptr);
		};

		if (body)
		{
			RunOtrReceiving (*entry, *body);

			// The contact may have stopped waiting for this. If its answer is merely late,
			// the query below makes the two sides exchange keys once more, which is harmless.
			using namespace std::chrono_literals;
			co_await 3s;

			const auto context = findContext (OTRL_INSTAG_BEST);
			const auto& actions = Entry2Action_.value (entry->GetQObject ());
			if ((context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED) ||
					!actions.ToggleOtr_ || !actions.ToggleOtr_->isChecked ())
				co_return;
		}
		else
		{
			// Without the message there is nothing to resume, and what is left of this handshake
			// on our side can win the collision with the contact's answer to the query.
			const auto master = findContext (OTRL_INSTAG_MASTER);
			for (auto context = master; context && context->m_context == master; context = context->next)
				if (context->auth.authstate != OTRL_AUTHSTATE_NONE)
					otrl_auth_clear (&context->auth);
		}

		SendOtrQuery (*entry);
	}

	void OtrHandler::SetOtrActionsState (ICLEntry& entry, bool enabled)
	{
		if (const auto& actions = Entry2Action_.value (entry.GetQObject ());
			actions.ToggleOtr_ && actions.ToggleOtrCtx_)
		{
			actions.ToggleOtr_->setChecked (enabled);
			actions.ToggleOtrCtx_->setChecked (enabled);
		}
	}

	Util::ContextTask<void> OtrHandler::HandleAuthRequested (Util::TrackingPointer<ICLEntry> entry)
	{
		co_await Util::AddContext { CoroContext_ };

		if (!Entry2Action_ [entry->GetQObject ()].ToggleOtr_->isChecked ())
		{
			if (co_await Util::Dialogs::Question (nullptr,
						"LeechCraft",
						tr ("You need to start a private conversation before authentication "
							"can take place. Do you want to start it?"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				co_return;

			if (co_await SetOtrState (entry, true) != OtrState::QuerySent)
				co_return;
		}

		if (!Auths_.contains (&*entry))
			CreateAuthForEntry (&*entry);

		const auto auth = Auths_.value (&*entry);
		QTimer::singleShot (0, auth, &Authenticator::Initiate);
	}

	void OtrHandler::StartAuth (ICLEntry *entry, SmpMethod method,
			const QString& questionStr, const QString& answerStr)
	{
		const auto acc = entry->GetParentAccount ();
		const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		const auto context = otrl_context_find (UserState_,
				SerializeString (entry->GetStrongestId ()).toUtf8 ().constData (),
				acc->GetAccountId ().ToUtf8 ().constData (),
				proto->GetProtocolId ().toUtf8 ().constData (),
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
}
}
}
