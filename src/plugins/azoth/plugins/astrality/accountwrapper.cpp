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

#include "accountwrapper.h"
#include <algorithm>
#include <PendingOperation>
#include <PendingStringList>
#include <PendingContacts>
#include <Connection>
#include <ContactManager>
#include <util/util.h>
#include <util/passutils.h>
#include "astralityutil.h"
#include "entrywrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	AccountWrapper::AccountWrapper (Tp::AccountPtr acc, QObject *parent)
	: QObject (parent)
	, A_ (acc)
	{
		connect (A_->setEnabled (true),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleEnabled (Tp::PendingOperation*)));
		connect (A_.data (),
				SIGNAL (currentPresenceChanged (Tp::Presence)),
				this,
				SLOT (handleCurrentPresence (Tp::Presence)));
		connect (A_.data (),
				SIGNAL (connectionStatusChanged (Tp::ConnectionStatus)),
				this,
				SLOT (handleConnStatusChanged (Tp::ConnectionStatus)));
		connect (A_.data (),
				SIGNAL (connectionChanged (Tp::ConnectionPtr)),
				this,
				SLOT (handleConnectionChanged (Tp::ConnectionPtr)));
	}

	QObject* AccountWrapper::GetObject ()
	{
		return this;
	}

	QObject* AccountWrapper::GetParentProtocol () const
	{
		return parent ();
	}

	IAccount::AccountFeatures AccountWrapper::GetAccountFeatures () const
	{
		return FRenamable;
	}

	QList<QObject*> AccountWrapper::GetCLEntries ()
	{
		QList<QObject*> result;
		Q_FOREACH (auto e, Entries_)
			result << e;
		return result;
	}

	QString AccountWrapper::GetAccountName () const
	{
		return A_->displayName ();
	}

	QString AccountWrapper::GetOurNick () const
	{
		QString result = A_->nickname ();
		if (result.isEmpty ())
			result = A_->displayName ();
		return result;
	}

	void AccountWrapper::RenameAccount (const QString& newName)
	{
	}

	QByteArray AccountWrapper::GetAccountID () const
	{
		return QString ("Astrality.%1.%2.%3")
				.arg (A_->cmName ())
				.arg (A_->protocolName ())
				.arg (A_->uniqueIdentifier ())
				.toUtf8 ();
	}

	QList<QAction*> AccountWrapper::GetActions () const
	{
		return QList<QAction*> ();
	}

	void AccountWrapper::QueryInfo (const QString&)
	{
	}

	void AccountWrapper::OpenConfigurationDialog ()
	{
	}

	EntryStatus AccountWrapper::GetState () const
	{
		const auto& pres = A_->currentPresence ();
		const State state = StateTelepathy2Azoth (pres.type ());
		return EntryStatus (state, pres.statusMessage ());
	}

	void AccountWrapper::ChangeState (const EntryStatus& status)
	{
		qDebug () << Q_FUNC_INFO << A_->connectsAutomatically () << A_->isEnabled () << A_->isValid () << A_->isValidAccount ();
		qDebug () << A_->parameters ();
		connect (A_->setRequestedPresence (Status2Telepathy (status)),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleRequestedPresenceFinish (Tp::PendingOperation*)));
	}

	void AccountWrapper::Authorize (QObject *obj)
	{
		auto w = qobject_cast<EntryWrapper*> (obj);
		w->ResendAuth (QString ());
		w->RerequestAuth (QString ());
	}

	void AccountWrapper::DenyAuth (QObject *obj)
	{
		auto w = qobject_cast<EntryWrapper*> (obj);
		w->RevokeAuth (QString ());
	}

	void AccountWrapper::RequestAuth (const QString& id,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		if (!A_->connection ())
		{
			qWarning () << Q_FUNC_INFO
					<< "connection isn't ready";
			return;
		}

		auto cm = A_->connection ()->contactManager ();
		auto req = cm->contactsForIdentifiers (QStringList (id));
		req->setProperty ("Astrality/Msg", msg);
		connect (req,
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleAuthRequestFinished (Tp::PendingOperation*)));
	}

	void AccountWrapper::RemoveEntry (QObject*)
	{
	}

	QObject* AccountWrapper::GetTransferManager () const
	{
		return 0;
	}

	Tp::ContactMessengerPtr AccountWrapper::GetMessenger (const QString& id)
	{
		if (!Messengers_.contains (id))
				Messengers_ [id] = Tp::ContactMessenger::create (A_, id);

		return Messengers_ [id];
	}

	void AccountWrapper::RemoveThis ()
	{
		connect (A_->remove (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleRemoved (Tp::PendingOperation*)));
	}

	void AccountWrapper::HandleAuth (bool failure)
	{
		const QString key = GetAccountID ().replace ('/', '_') + "." +
				A_->parameters () ["account"].toString ();
		const QString& msg = tr ("Enter password for account %1 with login %2:")
				.arg (A_->displayName ())
				.arg (A_->parameters () ["account"].toString ());

		const QString& pass = Util::GetPassword (key, msg, this, !failure);
		if (pass.isEmpty ())
			return;

		QVariantMap map;
		map ["password"] = pass;
		connect (A_->updateParameters (map, QStringList ()),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handlePasswordFixed (Tp::PendingOperation*)));
	}

	EntryWrapper* AccountWrapper::CreateEntry (Tp::ContactPtr c)
	{
		auto pos = std::find_if (Entries_.begin (), Entries_.end (),
			[c] (decltype (Entries_.front ()) e)
				{ return e->GetContact () == c; });
		if (pos != Entries_.end ())
			return *pos;

		auto w = new EntryWrapper (c, this);
		Entries_ << w;
		emit gotCLItems (QList<QObject*> () << w);

		connect (w,
				SIGNAL (itemSubscribed (QObject*, QString)),
				this,
				SIGNAL (itemSubscribed (QObject*, QString)));
		connect (w,
				SIGNAL (itemUnsubscribed (QObject*, QString)),
				this,
				SIGNAL (itemUnsubscribed (QObject*, QString)));
		connect (w,
				SIGNAL (itemCancelledSubscription (QObject*, QString)),
				this,
				SIGNAL (itemCancelledSubscription (QObject*, QString)));
		connect (w,
				SIGNAL (itemGrantedSubscription (QObject*, QString)),
				this,
				SIGNAL (itemGrantedSubscription (QObject*, QString)));

		return w;
	}

	void AccountWrapper::handleEnabled (Tp::PendingOperation *po)
	{
		qDebug () << Q_FUNC_INFO << po->isError ();
		if (po->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< po->errorName ()
					<< po->errorMessage ();
			emit gotEntity (Util::MakeNotification ("Azoth",
					tr ("Error enabling account %1: %2 (%3).")
						.arg (A_->displayName ())
						.arg (po->errorName ())
						.arg (po->errorMessage ()),
					PCritical_));
		}

		HandleAuth (false);
		handleConnectionChanged (A_->connection ());
	}

	void AccountWrapper::handleRemoved (Tp::PendingOperation *po)
	{
		if (po->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< po->errorName ()
					<< po->errorMessage ();
			emit gotEntity (Util::MakeNotification ("Azoth",
					tr ("Error removing account %1: %2 (%3).")
						.arg (A_->displayName ())
						.arg (po->errorName ())
						.arg (po->errorMessage ()),
					PCritical_));
			return;
		}

		emit removeFinished (this);
	}

	void AccountWrapper::handleConnStatusChanged (Tp::ConnectionStatus status)
	{
		qDebug () << Q_FUNC_INFO << status;
		qDebug () << A_->connectionStatusReason ();
		qDebug () << A_->connectionError () << A_->connectionErrorDetails ().allDetails ();

		const auto reason = A_->connectionStatusReason ();
		if (reason == Tp::ConnectionStatusReasonRequested)
			return;

		QString message;
		switch (reason)
		{
		case Tp::ConnectionStatusReasonNetworkError:
			message = tr ("network error");
			break;
		case Tp::ConnectionStatusReasonAuthenticationFailed:
			message = tr ("authentication failed");
			break;
		case Tp::ConnectionStatusReasonEncryptionError:
			message = tr ("encryption error");
			break;
		case Tp::ConnectionStatusReasonNameInUse:
			message = tr ("resource or name is already in use");
			break;
		case Tp::ConnectionStatusReasonCertNotProvided:
			message = tr ("certificate hasn't been provided");
			break;
		case Tp::ConnectionStatusReasonCertUntrusted:
			message = tr ("certificate is untrusted");
			break;
		case Tp::ConnectionStatusReasonCertExpired:
			message = tr ("certificate expired");
			break;
		case Tp::ConnectionStatusReasonCertNotActivated:
			message = tr ("certificate isn't activated");
			break;
		case Tp::ConnectionStatusReasonCertHostnameMismatch:
			message = tr ("hostname mismatch in certificate");
			break;
		case Tp::ConnectionStatusReasonCertFingerprintMismatch:
			message = tr ("certificate fingerprint mismatch");
			break;
		case Tp::ConnectionStatusReasonCertSelfSigned:
			message = tr ("certificate is self-signed");
			break;
		case Tp::ConnectionStatusReasonCertOtherError:
			message = tr ("other certificate error");
			break;
		case Tp::ConnectionStatusReasonCertRevoked:
			message = tr ("certificate is revoked");
			break;
		case Tp::ConnectionStatusReasonCertInsecure:
			message = tr ("certificate is insecure");
			break;
		case Tp::ConnectionStatusReasonCertLimitExceeded:
			message = tr ("certificate length limit is exceeded");
			break;
		default:
			message = tr ("other error");
			break;
		}

		const QString& dbgMsg = A_->connectionErrorDetails ().debugMessage ();
		QString body = tr ("Connection error for account %1: %2.")
				.arg (A_->displayName ())
				.arg (message);
		if (!dbgMsg.isEmpty ())
			body += " " + tr ("Backend message: %1.").arg (dbgMsg);

		emit gotEntity (Util::MakeNotification ("Azoth", body, PCritical_));

		if (reason == Tp::ConnectionStatusReasonAuthenticationFailed)
			HandleAuth (true);
	}

	void AccountWrapper::handleConnectionChanged (Tp::ConnectionPtr conn)
	{
		qDebug () << Q_FUNC_INFO << conn;
		if (!Entries_.isEmpty ())
		{
			emit removedCLItems (GetCLEntries ());
			qDeleteAll (Entries_);
			Entries_.clear ();
		}

		if (!conn)
			return;

		auto cm = conn->contactManager ().data ();
		connect (cm,
				SIGNAL (presencePublicationRequested (Tp::Contacts)),
				this,
				SLOT (handlePresencePubRequested (Tp::Contacts)));
		connect (cm,
				SIGNAL (stateChanged (Tp::ContactListState)),
				this,
				SLOT (handleCMStateChanged (Tp::ContactListState)));
		connect (cm,
				SIGNAL (allKnownContactsChanged (Tp::Contacts, Tp::Contacts, Tp::Channel::GroupMemberChangeDetails)),
				this,
				SLOT (handleKnownContactsChanged (Tp::Contacts, Tp::Contacts, Tp::Channel::GroupMemberChangeDetails)));

		handleCMStateChanged (cm->state ());
	}

	void AccountWrapper::handlePasswordFixed (Tp::PendingOperation *po)
	{
		qWarning () << Q_FUNC_INFO;
		if (po->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< po->errorName ()
					<< po->errorMessage ();
			emit gotEntity (Util::MakeNotification ("Azoth",
					tr ("Failed to update password for account %1: %2 (%3).")
						.arg (A_->displayName ())
						.arg (po->errorName ())
						.arg (po->errorMessage ()),
					PCritical_));
			return;
		}

		A_->setRequestedPresence (A_->requestedPresence ());
	}

	void AccountWrapper::handleRequestedPresenceFinish (Tp::PendingOperation *po)
	{
		qDebug () << Q_FUNC_INFO << A_->currentPresence ().type () << A_->currentPresence ().status () << A_->currentPresence ().statusMessage ();
		qDebug () << A_->connectionStatus () << A_->connectionStatusReason ();
		qDebug () << A_->connectionError () << A_->connectionErrorDetails ().allDetails ();
		if (po->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< po->errorName ()
					<< po->errorMessage ();
			emit gotEntity (Util::MakeNotification ("Azoth",
					tr ("Error changing state for account %1: %2 (%3).")
						.arg (A_->displayName ())
						.arg (po->errorName ())
						.arg (po->errorMessage ()),
					PCritical_));
		}
	}

	void AccountWrapper::handleCurrentPresence (Tp::Presence pres)
	{
		qDebug () << Q_FUNC_INFO << pres.type ();
		emit statusChanged (GetState ());
	}

	void AccountWrapper::handlePresencePubRequested (Tp::Contacts contacts)
	{
		qDebug () << Q_FUNC_INFO << contacts.size ();
		Q_FOREACH (Tp::ContactPtr c, contacts)
		{
			qDebug () << c->alias () << c->groups () << c->id ();
			auto w = CreateEntry (c);
			emit authorizationRequested (w, QString ());
		}
	}

	void AccountWrapper::handleCMStateChanged (Tp::ContactListState state)
	{
		qDebug () << Q_FUNC_INFO << state;

		auto cm = A_->connection ()->contactManager ();
		auto contacts = cm->allKnownContacts ();
		qDebug () << Q_FUNC_INFO << contacts.size () << "contacts";

		if (state != Tp::ContactListStateSuccess)
			return;

		Q_FOREACH (Tp::ContactPtr c, contacts)
		{
			qDebug () << c->alias () << c->groups () << c->id ();
			CreateEntry (c);
		}
	}

	void AccountWrapper::handleKnownContactsChanged (Tp::Contacts added,
			Tp::Contacts removed, Tp::Channel::GroupMemberChangeDetails)
	{
		qDebug () << Q_FUNC_INFO << added.size () << removed.size ();
		Q_FOREACH (Tp::ContactPtr c, added)
			if (std::find_if (Entries_.begin (), Entries_.end (),
					[c] (decltype (Entries_.front ()) e)
						{ return c->id () == e->GetHumanReadableID (); }) == Entries_.end ())
				CreateEntry (c);

		Q_FOREACH (Tp::ContactPtr c, removed)
		{
			auto pos = std::find_if (Entries_.begin (), Entries_.end (),
					[c] (decltype (Entries_.front ()) e)
						{ return c->id () == e->GetHumanReadableID (); });
			if (pos == Entries_.end ())
			{
				qWarning () << Q_FUNC_INFO
						<< "no contact for removed contact"
						<< c->id ();
				continue;
			}

			auto our = *pos;
			emit removedCLItems (QList<QObject*> () << our);
			our->deleteLater ();
			Entries_.erase (pos);
		}
	}

	void AccountWrapper::handleAuthRequestFinished (Tp::PendingOperation *po)
	{
		auto pc = qobject_cast<Tp::PendingContacts*> (po);
		qDebug () << Q_FUNC_INFO << pc->contacts ().size ();

		const QString& msg = po->property ("Astrality/Msg").toString ();
		Q_FOREACH (Tp::ContactPtr c, pc->contacts ())
		{
			qDebug () << c->alias () << c->id ();
			CreateEntry (c);

			connect (c->requestPresenceSubscription (msg),
					SIGNAL (finished (Tp::PendingOperation*)),
					this,
					SLOT (handleAuthRequestSent (Tp::PendingOperation*)));
		}
	}

	void AccountWrapper::handleAuthRequestSent (Tp::PendingOperation *po)
	{
		if (!po->isError ())
			return;

		qWarning () << Q_FUNC_INFO
				<< po->errorName ()
				<< po->errorMessage ();
		emit gotEntity (Util::MakeNotification ("Azoth",
				tr ("Failed to request authorization: %1 (%2).")
					.arg (po->errorName ())
					.arg (po->errorMessage ()),
				PCritical_));
	}
}
}
}
