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

#include "protowrapper.h"
#include <QIcon>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingReady>

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	ProtoWrapper::ProtoWrapper (Tp::ConnectionManagerPtr cm,
			const QString& protoName, QObject *parent)
	: QObject (parent)
	, CM_ (cm)
	, ProtoName_ (protoName)
	{
		auto accf = Tp::AccountFactory::create (QDBusConnection::sessionBus (),
				Tp::Account::FeatureCore);
		AM_ = Tp::AccountManager::create (accf);

		connect (AM_->becomeReady (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleAMReady (Tp::PendingOperation*)));
		connect (AM_.data (),
				SIGNAL (newAccount (Tp::AccountPtr)),
				this,
				SLOT (handleNewAccount (Tp::AccountPtr)));
	}

	QObject* ProtoWrapper::GetObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures ProtoWrapper::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> ProtoWrapper::GetRegisteredAccounts ()
	{
		return QList<QObject*> ();
	}

	QObject* ProtoWrapper::GetParentProtocolPlugin () const
	{
		return parent ();
	}

	QString ProtoWrapper::GetProtocolName () const
	{
		return ProtoName_;
	}

	QIcon ProtoWrapper::GetProtocolIcon () const
	{
		return QIcon ();
	}

	QByteArray ProtoWrapper::GetProtocolID () const
	{
		QByteArray res = ProtoName_.toUtf8 ().toLower ();
		if (res == "jabber")
			res = "xmpp";
		return res;
	}

	QList<QWidget*> ProtoWrapper::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return QList<QWidget*> ();
	}

	QWidget* ProtoWrapper::GetMUCJoinWidget ()
	{
		return 0;
	}

	void ProtoWrapper::RegisterAccount (const QString&, const QList<QWidget*>&)
	{
	}

	void ProtoWrapper::RemoveAccount (QObject*)
	{
	}

	void ProtoWrapper::handleAMReady (Tp::PendingOperation *po)
	{
		if (po->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< ProtoName_
					<< po->errorName ()
					<< po->errorMessage ();
		}
		qDebug () << Q_FUNC_INFO << ProtoName_ << AM_->supportedAccountProperties ();

		Q_FOREACH (auto acc, AM_->allAccounts ())
			handleNewAccount (acc);
	}

	void ProtoWrapper::handleNewAccount (Tp::AccountPtr)
	{
	}
}
}
}
