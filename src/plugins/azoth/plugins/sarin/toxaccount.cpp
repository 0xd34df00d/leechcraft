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
#include <util/sll/qtutil.h>
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
		return Tox_ ? Status_ : EntryStatus {};
	}

	void ToxAccount::ChangeState (const EntryStatus& status)
	{
		if (status.State_ == SOffline)
		{
			if (Tox_)
			{
				emit statusChanged (status);
				Tox_.reset ();
				emit threadChanged (Tox_);
			}
			return;
		}

		if (!Tox_)
			InitThread (status);
		else
			Tox_->Run (&ToxW::SetStatus, status);
	}

	void ToxAccount::Authorize (QObject *entryObj)
	{
		if (!Tox_)
			return;

		const auto entry = qobject_cast<ToxContact*> (entryObj);
		Tox_->Run (&ToxW::AddFriendNoRequest, entry->GetPubKey ());
	}

	void ToxAccount::DenyAuth (QObject*)
	{
	}

	Util::ContextTask<void> ToxAccount::RunRequestAuth (QString toxId, QString msg)
	{
		co_await Util::AddContextObject { *this };

		auto tox = Tox_;
		if (!tox)
			co_return;

		const auto addResult = co_await tox->Run (&ToxW::AddFriend, toxId.toUtf8 (), msg);
		const auto friendNum = co_await Util::WithHandler (addResult,
				[] (const ToxError<AddFriendError>& error)
				{
					qWarning () << "cannot add friend:" << error.Message_;
					const auto& e = Util::MakeNotification ("Azoth Sarin"_qs,
							tr ("Unable to add contact: %1.").arg (ToUserString (error.Code_)),
							Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		const auto resolveResult = co_await tox->Run (&ToxW::ResolveFriend, friendNum);
		Util::Visit (resolveResult,
				[this] (const ToxW::FriendInfo& info)
				{
					qDebug () << info.Pubkey_ << info.Name_;
					if (!Contacts_.contains (info.Pubkey_))
						InitEntry (info.Pubkey_);

					const auto entry = Contacts_.value (info.Pubkey_);
					entry->SetEntryName (info.Name_);
					entry->SetStatus (info.Status_);
				},
				[] (const ToxError<FriendQueryError>& error)
				{
					qWarning () << "cannot get friend info:" << error.Message_;
					const auto& e = Util::MakeNotification ("Azoth Sarin"_qs,
							tr ("Unable to resolve contact info: %1.").arg (ToUserString (error.Code_)),
							Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
	}

	void ToxAccount::RequestAuth (const QString& toxId, const QString& msg, const QString&, const QStringList&)
	{
		RunRequestAuth (toxId, msg);
	}

	Util::ContextTask<void> ToxAccount::RunRemoveEntry (ToxContact *entry)
	{
		co_await Util::AddContextObject { *this };
		if (!Tox_)
			co_return;

		const auto& pkey = entry->GetHumanReadableID ().toUtf8 ();
		if (co_await Tox_->Run (&ToxW::RemoveFriend, pkey))
			HandleRemovedFriend (pkey);
	}

	void ToxAccount::RemoveEntry (QObject *entryObj)
	{
		if (const auto entry = qobject_cast<ToxContact*> (entryObj))
			RunRemoveEntry (entry);
		else
			qWarning () << entryObj << "is not a ToxContact";
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
		if (!Tox_)
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

		const auto callMgr = Tox_->GetCallManager ();
		const auto call = new AudioCall { entry, callMgr, AudioCall::DOut };
		emit called (call);
		Util::Sequence (entry, Tox_->ResolveFriendId (entry->GetPubKey ())) >>
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

	Util::ContextTask<void> ToxAccount::SetTypingState (QByteArray pkey, bool isTyping)
	{
		co_await Util::AddContextObject { *this };
		if (!Tox_)
			co_return;

		const auto num = co_await Tox_->Run (&ToxW::ResolveFriendNum, pkey);
		if (!num)
			co_return;
		const auto result = co_await Tox_->RunWithError (&tox_self_set_typing, *num, isTyping);
		if (const auto err = result.MaybeLeft ())
			qWarning () << "cannot set typing to" << isTyping << ":" << *err << tox_err_set_typing_to_string (*err);
	}

	Util::ContextTask<void> ToxAccount::InitThread (EntryStatus status)
	{
		co_await Util::AddContextObject { *this };

		Tox_ = std::make_shared<ToxRunner> (ToxW::InitContext { Nick_, ToxState_, ToxConfig_ });

		connect (&*Tox_,
				&ToxRunner::statusChanged,
				this,
				[this] (const EntryStatus& s)
				{
					Status_ = s;
					emit statusChanged (s);
				});
		connect (Tox_.get (),
				&ToxRunner::toxStateChanged,
				this,
				[this] (const QByteArray& toxState)
				{
					ToxState_ = toxState;
					emit accountChanged (this);
				});
		connect (Tox_.get (),
				&ToxRunner::gotFriendRequest,
				this,
				&ToxAccount::HandleGotFriendRequest);
		connect (Tox_.get (),
				&ToxRunner::friendNameChanged,
				this,
				&ToxAccount::HandleFriendNameChanged);
		connect (Tox_.get (),
				&ToxRunner::friendStatusChanged,
				this,
				&ToxAccount::HandleFriendStatusChanged);
		connect (Tox_.get (),
				&ToxRunner::friendTypingChanged,
				this,
				&ToxAccount::HandleFriendTypingChanged);

		qDebug () << "initializing...";
		const auto initResult = co_await Tox_->Run (&ToxW::Init, status);
		Util::Visit (initResult,
				[this] (Util::Void)
				{
					qDebug () << "done!";
					emit threadChanged (Tox_);
#ifdef ENABLE_MEDIACALLS
					const auto callManager = Tox_->GetCallManager ();
					connect (callManager,
							&CallManager::gotIncomingCall,
							this,
							&ToxAccount::HandleIncomingCall);
#endif
				},
				[this] (const ToxError<InitError>& error)
				{
					qWarning () << "failed to init Tox:" << error;
					Tox_.reset ();

					// TODO human-readable message for the code
					const auto& e = Util::MakeNotification ("Azoth Sarin"_qs,
							tr ("Unable to initialize Tox: %1 (%2).").arg (static_cast<int> (error.Code_)).arg (error.Message_),
							Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
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

		const auto call = new AudioCall { entry, Tox_->GetCallManager (), AudioCall::DIn };
		call->SetCallIdx (callIdx);
		emit called (call);
#else
		Q_UNUSED (pubkey)
		Q_UNUSED (callIdx)
#endif
	}

	Util::ContextTask<void> ToxAccount::HandleToxIdRequested ()
	{
		co_await Util::AddContextObject { *this };
		if (!Tox_)
			co_return;

		const auto dialog = new ShowToxIdDialog { tr ("Fetching self Tox ID...") };
		dialog->show ();
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		co_await Util::AddContextObject { *dialog };

		const auto& toxId = co_await Tox_->Run (&ToxW::GetToxId);
		dialog->setToxId (QString::fromLatin1 (toxId));
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
			qWarning () << "unknown friend name change" << id << "; new name:" << newName;
			return;
		}

		Contacts_.value (id)->SetEntryName (newName);
	}

	void ToxAccount::HandleFriendStatusChanged (const QByteArray& pubkey, const EntryStatus& status)
	{
		if (!Contacts_.contains (pubkey))
		{
			qWarning () << "unknown friend status" << pubkey;
			return;
		}

		Contacts_.value (pubkey)->SetStatus (status);
	}

	void ToxAccount::HandleFriendTypingChanged (const QByteArray& pubkey, bool isTyping)
	{
		if (!Contacts_.contains (pubkey))
		{
			qWarning () << "unknown friend status" << pubkey;
			return;
		}

		Contacts_.value (pubkey)->SetTyping (isTyping);
	}

	void ToxAccount::HandleInMessage (const QByteArray& pubkey, const QString& body)
	{
		if (!Contacts_.contains (pubkey))
		{
			qWarning () << "unknown pubkey for message" << body << pubkey;
			InitEntry (pubkey);
		}

		const auto contact = Contacts_.value (pubkey);
		const auto msg = new ChatMessage { body, IMessage::Direction::In, contact };
		msg->Store ();
	}
}
