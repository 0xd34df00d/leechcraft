/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toxaccount.h"
#include <QUuid>
#include <QAction>
#include <QDataStream>
#include <QtDebug>
#include <tox/tox.h>
#include <util/xpc/util.h>
#include <util/sll/slotclosure.h>
#include <util/sll/functional.h>
#include <util/threads/futures.h>
#include <interfaces/core/ientitymanager.h>
#include "toxprotocol.h"
#include "toxthread.h"
#include "showtoxiddialog.h"
#include "toxcontact.h"
#include "messagesmanager.h"
#include "chatmessage.h"
#include "accountconfigdialog.h"
#include "util.h"
#include "filetransfermanager.h"

#ifdef ENABLE_MEDIACALLS
#include "audiocall.h"
#endif

namespace LC::Azoth::Sarin
{
	ToxAccount::ToxAccount (const QByteArray& uid, const QString& name, ToxProtocol *parent)
	: QObject { parent }
	, Proto_ { parent }
	, UID_ { uid }
	, Name_ { name }
	, ActionGetToxId_ { new QAction { tr ("Get Tox ID"), this } }
	, MsgsMgr_ { new MessagesManager { this } }
	, XferMgr_ { new FileTransferManager { this } }
	{
		connect (ActionGetToxId_,
				&QAction::triggered,
				this,
				&ToxAccount::HandleToxIdRequested);

		connect (MsgsMgr_,
				&MessagesManager::gotMessage,
				this,
				&ToxAccount::HandleInMessage);

		connect (this,
				&ToxAccount::threadChanged,
				XferMgr_,
				&FileTransferManager::HandleToxThreadChanged);
	}

	ToxAccount::ToxAccount (const QString& name, ToxProtocol *parent)
	: ToxAccount { QUuid::createUuid ().toByteArray (), name, parent }
	{
	}

	QByteArray ToxAccount::Serialize ()
	{
		QByteArray ba;
		QDataStream str { &ba, QIODevice::WriteOnly };
		str << static_cast<quint8> (2)
				<< UID_
				<< Name_
				<< Nick_
				<< ToxState_
				<< ToxConfig_;

		return ba;
	}

	ToxAccount* ToxAccount::Deserialize (const QByteArray& data, ToxProtocol *proto)
	{
		QDataStream str { data };
		quint8 version = 0;
		str >> version;
		if (version < 1 || version > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return nullptr;
		}

		QByteArray uid;
		QString name;
		str >> uid
				>> name;

		const auto acc = new ToxAccount { uid, name, proto };
		str >> acc->Nick_
				>> acc->ToxState_;

		if (version >= 2)
			str >> acc->ToxConfig_;

		return acc;
	}

	void ToxAccount::SetNickname (const QString& nickname)
	{
		if (nickname == Nick_)
			return;

		Nick_ = nickname;
		emit accountChanged (this);
	}

	ToxContact* ToxAccount::GetByAzothId (const QString& azothId) const
	{
		const auto& localId = azothId.section ('/', 1).toUtf8 ();
		if (!Contacts_.contains (localId))
			qWarning () << Q_FUNC_INFO
					<< "unable to find entry for Azoth ID"
					<< azothId;
		return Contacts_.value (localId);
	}

	ToxContact* ToxAccount::GetByPubkey (const QByteArray& pubkey) const
	{
		return Contacts_.value (pubkey);
	}

	QObject* ToxAccount::GetQObject ()
	{
		return this;
	}

	QObject* ToxAccount::GetParentProtocol () const
	{
		return Proto_;
	}

	IAccount::AccountFeatures ToxAccount::GetAccountFeatures () const
	{
		return FRenamable;
	}

	QList<QObject*> ToxAccount::GetCLEntries ()
	{
		QList<QObject*> result;
		std::copy (Contacts_.begin (), Contacts_.end (), std::back_inserter (result));
		return result;
	}

	QString ToxAccount::GetAccountName () const
	{
		return Name_;
	}

	QString ToxAccount::GetOurNick () const
	{
		return Nick_.isEmpty () ? Name_ : Nick_;
	}

	void ToxAccount::RenameAccount (const QString& name)
	{
		if (name == Name_)
			return;

		Name_ = name;
		emit accountRenamed (Name_);
		emit accountChanged (this);
	}

	QByteArray ToxAccount::GetAccountID () const
	{
		return UID_;
	}

	QList<QAction*> ToxAccount::GetActions () const
	{
		return { ActionGetToxId_ };
	}

	void ToxAccount::OpenConfigurationDialog ()
	{
		auto configDialog = new AccountConfigDialog;
		configDialog->setAttribute (Qt::WA_DeleteOnClose);
		configDialog->show ();

		configDialog->SetConfig (ToxConfig_);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, configDialog] { HandleConfigAccepted (configDialog); },
			configDialog,
			SIGNAL (accepted ()),
			configDialog
		};
	}

	EntryStatus ToxAccount::GetState () const
	{
		return Thread_ ? Thread_->GetStatus () : EntryStatus {};
	}

	void ToxAccount::ChangeState (const EntryStatus& status)
	{
		if (status.State_ == SOffline)
		{
			if (Thread_ && Thread_->IsStoppable ())
			{
				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[status, guard = Thread_, this] { emit statusChanged (status); },
					Thread_.get (),
					SIGNAL (finished ()),
					Thread_.get ()
				};
				Thread_->Stop ();
			}
			Thread_.reset ();
			emit threadChanged (Thread_);
			return;
		}

		if (!Thread_)
			InitThread (status);
		else
			Thread_->SetStatus (status);
	}

	void ToxAccount::Authorize (QObject *entryObj)
	{
		if (!Thread_)
			return;

		const auto entry = qobject_cast<ToxContact*> (entryObj);
		Thread_->AddFriend (entry->GetPubKey ());
	}

	void ToxAccount::DenyAuth (QObject*)
	{
	}

	void ToxAccount::RequestAuth (const QString& toxId, const QString& msg, const QString&, const QStringList&)
	{
		if (!Thread_)
			return;

		auto iem = Proto_->GetCoreProxy ()->GetEntityManager ();
		Util::Sequence (this, Thread_->AddFriend (toxId.toLatin1 (), msg)) >>
				[iem] (ToxThread::AddFriendResult result)
				{
					const auto notify = [iem] (Priority prio, const QString& text)
					{
						iem->HandleEntity (Util::MakeNotification ("Azoth Sarin", text, prio));
					};

					switch (result)
					{
					case ToxThread::AddFriendResult::Added:
						return;
					case ToxThread::AddFriendResult::InvalidId:
						notify (Priority::Critical, tr ("Friend was not added: invalid Tox ID."));
						return;
					case ToxThread::AddFriendResult::TooLong:
						notify (Priority::Critical, tr ("Friend was not added: too long greeting message."));
						return;
					case ToxThread::AddFriendResult::NoMessage:
						notify (Priority::Critical, tr ("Friend was not added: no message."));
						return;
					case ToxThread::AddFriendResult::OwnKey:
						notify (Priority::Critical, tr ("Why would you add yourself as friend?"));
						return;
					case ToxThread::AddFriendResult::AlreadySent:
						notify (Priority::Warning, tr ("Friend request has already been sent."));
						return;
					case ToxThread::AddFriendResult::BadChecksum:
						notify (Priority::Critical, tr ("Friend was not added: bad Tox ID checksum."));
						return;
					case ToxThread::AddFriendResult::NoSpam:
						notify (Priority::Critical, tr ("Friend was not added: nospam value has been changed. Get a freshier ID!"));
						return;
					case ToxThread::AddFriendResult::NoMem:
						notify (Priority::Critical, tr ("Friend was not added: no memory (but how do you see this in that case?)."));
						return;
					case ToxThread::AddFriendResult::Unknown:
						notify (Priority::Critical, tr ("Friend was not added because of some unknown error."));
						return;
					}
				};
	}

	void ToxAccount::RemoveEntry (QObject *entryObj)
	{
		if (!Thread_)
			return;

		const auto entry = qobject_cast<ToxContact*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is not a ToxContact";
			return;
		}

		Thread_->RemoveFriend (entry->GetHumanReadableID ().toUtf8 ());
	}

	ISupportMediaCalls::MediaCallFeatures ToxAccount::GetMediaCallFeatures () const
	{
#ifdef ENABLE_MEDIACALLS
		return MCFSupportsAudioCalls;
#else
		return MCFNoFeatures;
#endif
	}

	QObject* ToxAccount::Call (const QString& id, const QString&)
	{
#ifdef ENABLE_MEDIACALLS
		if (!Thread_)
		{
			qWarning () << Q_FUNC_INFO
					<< "thread is not running";
			return nullptr;
		}

		const auto entry = GetByAzothId (id);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find contact by"
					<< id;
			return nullptr;
		}

		const auto callMgr = Thread_->GetCallManager ();
		const auto call = new AudioCall { entry, callMgr, AudioCall::DOut };
		emit called (call);
		Util::Sequence (entry, Thread_->ResolveFriendId (entry->GetPubKey ())) >>
				Util::BindMemFn (&AudioCall::SetCallIdx, call);
		return call;
#else
		Q_UNUSED (id)
		return nullptr;
#endif
	}

	QObject* ToxAccount::GetTransferManager () const
	{
		return XferMgr_;
	}

	void ToxAccount::SendMessage (const QByteArray& pkey, ChatMessage *message)
	{
		MsgsMgr_->SendMessage (pkey, message);
	}

	void ToxAccount::SetTypingState (const QByteArray& pkey, bool isTyping)
	{
		if (!Thread_)
			return;

		Thread_->ScheduleFunction ([pkey, isTyping] (Tox *tox)
				{
					const auto num = GetFriendId (tox, pkey);
					if (!num)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to find user ID for"
								<< pkey;
						return;
					}

					TOX_ERR_SET_TYPING error {};
					if (!tox_self_set_typing (tox, *num, isTyping, &error))
						qWarning () << Q_FUNC_INFO
								<< "cannot set typing state to"
								<< isTyping
								<< "because of error"
								<< error;
				});
	}

	void ToxAccount::InitThread (const EntryStatus& status)
	{
		Thread_ = std::make_shared<ToxThread> (Nick_, ToxState_, ToxConfig_);
		Thread_->SetStatus (status);
		connect (Thread_.get (),
				&ToxThread::statusChanged,
				this,
				&ToxAccount::statusChanged);
		connect (Thread_.get (),
				&ToxThread::toxStateChanged,
				this,
				[this] (const QByteArray& toxState)
				{
					ToxState_ = toxState;
					emit accountChanged (this);
				});
		connect (Thread_.get (),
				&ToxThread::gotFriendRequest,
				this,
				&ToxAccount::HandleGotFriendRequest);
		connect (Thread_.get (),
				&ToxThread::gotFriend,
				this,
				&ToxAccount::HandleGotFriend);
		connect (Thread_.get (),
				&ToxThread::friendNameChanged,
				this,
				&ToxAccount::HandleFriendNameChanged);
		connect (Thread_.get (),
				&ToxThread::friendStatusChanged,
				this,
				&ToxAccount::HandleFriendStatusChanged);
		connect (Thread_.get (),
				&ToxThread::friendTypingChanged,
				this,
				&ToxAccount::HandleFriendTypingChanged);
		connect (Thread_.get (),
				&ToxThread::removedFriend,
				this,
				&ToxAccount::HandleRemovedFriend);

		connect (Thread_.get (),
				&ToxThread::fatalException,
				this,
				&ToxAccount::HandleThreadFatalException);

		connect (Thread_.get (),
				&ToxThread::toxCreated,
				this,
				&ToxAccount::HandleThreadReady);

		emit threadChanged (Thread_);

		Thread_->start (QThread::IdlePriority);
	}

	void ToxAccount::InitEntry (const QByteArray& pkey)
	{
		const auto entry = new ToxContact { pkey, this };
		Contacts_ [pkey] = entry;

		emit gotCLItems ({ entry });
	}

	void ToxAccount::HandleConfigAccepted (AccountConfigDialog *dialog)
	{
		const auto& config = dialog->GetConfig ();
		if (config == ToxConfig_)
			return;

		ToxConfig_ = config;
		emit accountChanged (this);
	}

	void ToxAccount::HandleThreadReady ()
	{
		if (!Thread_)
			return;

#ifdef ENABLE_MEDIACALLS
		const auto callManager = Thread_->GetCallManager ();
		connect (callManager,
				&CallManager::gotIncomingCall,
				this,
				&ToxAccount::HandleIncomingCall);
#endif
	}

	void ToxAccount::HandleIncomingCall (const QByteArray& pubkey, int32_t callIdx)
	{
#ifdef ENABLE_MEDIACALLS
		qDebug () << Q_FUNC_INFO << pubkey << callIdx;
		const auto entry = Contacts_.value (pubkey);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot find entry by pubkey"
					<< pubkey;
			return;
		}

		const auto call = new AudioCall { entry, Thread_->GetCallManager (), AudioCall::DIn };
		call->SetCallIdx (callIdx);
		emit called (call);
#else
		Q_UNUSED (pubkey)
		Q_UNUSED (callIdx)
#endif
	}

	void ToxAccount::HandleToxIdRequested ()
	{
		if (!Thread_)
			return;

		auto dialog = new ShowToxIdDialog { tr ("Fetching self Tox ID...") };
		dialog->show ();
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		Util::Sequence (this, Thread_->GetToxId ()) >>
				[dialog] (const QByteArray& id) { dialog->setToxId (QString::fromLatin1 (id)); };
	}

	void ToxAccount::HandleGotFriend (qint32 id)
	{
		Util::Sequence (this, Thread_->ResolveFriend (id)) >>
				[this] (const ToxThread::FriendInfo& info)
				{
					try
					{
						qDebug () << Q_FUNC_INFO << info.Pubkey_ << info.Name_;
						if (!Contacts_.contains (info.Pubkey_))
							InitEntry (info.Pubkey_);

						const auto entry = Contacts_.value (info.Pubkey_);
						entry->SetEntryName (info.Name_);

						entry->SetStatus (info.Status_);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "cannot get friend info:"
								<< e.what ();
					}
				};
	}

	void ToxAccount::HandleGotFriendRequest (const QByteArray& pubkey, const QString& msg)
	{
		if (!Contacts_.contains (pubkey))
			InitEntry (pubkey);

		emit authorizationRequested (Contacts_.value (pubkey), msg.trimmed ());
	}

	void ToxAccount::HandleRemovedFriend (const QByteArray& pubkey)
	{
		if (!Contacts_.contains (pubkey))
			return;

		const auto item = Contacts_.take (pubkey);
		emit removedCLItems ({ item });
		item->deleteLater ();
	}

	void ToxAccount::HandleFriendNameChanged (const QByteArray& id, const QString& newName)
	{
		if (!Contacts_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown friend name change"
					<< id
					<< "; new name:"
					<< newName;
			return;
		}

		Contacts_.value (id)->SetEntryName (newName);
	}

	void ToxAccount::HandleFriendStatusChanged (const QByteArray& pubkey, const EntryStatus& status)
	{
		if (!Contacts_.contains (pubkey))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown friend status"
					<< pubkey;
			return;
		}

		Contacts_.value (pubkey)->SetStatus (status);
	}

	void ToxAccount::HandleFriendTypingChanged (const QByteArray& pubkey, bool isTyping)
	{
		if (!Contacts_.contains (pubkey))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown friend status"
					<< pubkey;
			return;
		}

		Contacts_.value (pubkey)->SetTyping (isTyping);
	}

	void ToxAccount::HandleInMessage (const QByteArray& pubkey, const QString& body)
	{
		if (!Contacts_.contains (pubkey))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown pubkey for message"
					<< body
					<< pubkey;
			InitEntry (pubkey);
		}

		const auto contact = Contacts_.value (pubkey);
		const auto msg = new ChatMessage { body, IMessage::Direction::In, contact };
		msg->Store ();
	}

	void ToxAccount::HandleThreadFatalException (const Util::QtException_ptr& e)
	{
		qWarning () << Q_FUNC_INFO
				<< e->what ();

		emit statusChanged ({ SError, e->what () });
	}
}
