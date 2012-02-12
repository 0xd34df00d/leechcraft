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
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingStringList>
#include <TelepathyQt/Connection>
#include <util/util.h>
#include <util/passutils.h>
#include "astralityutil.h"

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
		return QList<QObject*> ();
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

	void AccountWrapper::Authorize (QObject*)
	{
	}

	void AccountWrapper::DenyAuth (QObject*)
	{
	}

	void AccountWrapper::RequestAuth (const QString&,
			const QString&, const QString&, const QStringList&)
	{
	}

	void AccountWrapper::RemoveEntry (QObject*)
	{
	}

	QObject* AccountWrapper::GetTransferManager () const
	{
		return 0;
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
		case Tp::ConnectionStatusReasonAuthenticationFailed:
			message = tr ("authentication failed");
			break;
		case Tp::ConnectionStatusReasonNetworkError:
			message = tr ("network error");
			break;
		case Tp::ConnectionStatusReasonCertExpired:
			message = tr ("certificate expirted");
			break;
		// TODO recognize more
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
}
}
}
