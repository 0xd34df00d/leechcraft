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
#include <TelepathyQt/PendingAccount>
#include <util/util.h>
#include "accountregfirstpage.h"
#include "accountwrapper.h"

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
	, ProtoInfo_ (CM_->protocol (ProtoName_))
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
		ProtocolFeatures features = PFNone;
		if (ProtoInfo_.canRegister ())
			features |= PFSupportsInBandRegistration;
		return features;
	}

	QList<QObject*> ProtoWrapper::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		Q_FOREACH (auto acc, Accounts_)
			result << acc;
		return result;
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
		return QIcon::fromTheme (ProtoInfo_.iconName ());
	}

	QByteArray ProtoWrapper::GetProtocolID () const
	{
		QByteArray res = ProtoName_.toUtf8 ().toLower ();
		if (res == "jabber")
			res = "xmpp";
		return res;
	}

	QList<QWidget*> ProtoWrapper::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions opts)
	{
		const bool reg = opts & AAORegisterNewAccount;
		auto fpage = new AccountRegFirstPage (ProtoInfo_, reg);
		if (reg)
			fpage->setProperty ("Astrality/RegisterNew", true);
		return QList<QWidget*> () << fpage;
	}

	QWidget* ProtoWrapper::GetMUCJoinWidget ()
	{
		return 0;
	}

	void ProtoWrapper::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		auto fpg = qobject_cast<AccountRegFirstPage*> (widgets.value (0));
		if (!fpg)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect first page"
					<< widgets;
			return;
		}

		QVariantMap params;
		auto chSet = [&params, &ProtoInfo_] (QString param, QVariant value)
		{
			if (ProtoInfo_.hasParameter (param))
				params [param] = value;
		};
		chSet ("account", fpg->GetAccountID ());
		if (!fpg->GetServer ().isEmpty ())
			chSet ("server", fpg->GetServer ());
		if (fpg->GetPort ())
			chSet ("port", fpg->GetPort ());
		chSet ("require-encryption", fpg->ShouldRequireEncryption ());
		if (fpg->property ("Astrality/RegisterNew").toBool ())
		{
			chSet ("password", fpg->GetPassword ());
			chSet ("register", true);
		}

		connect (AM_->createAccount (CM_->name (), ProtoName_, name, params, QVariantMap ()),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleAccountCreated (Tp::PendingOperation*)));
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

	void ProtoWrapper::handleAccountCreated (Tp::PendingOperation *po)
	{
		if (po->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< po->errorName ()
					<< po->errorMessage ();

			emit gotEntity (Util::MakeNotification ("Azoth",
						tr ("Failed to create account: %1 (%2).")
							.arg (po->errorName ())
							.arg (po->errorMessage ()),
						PCritical_));
			return;
		}

		auto pacc = qobject_cast<Tp::PendingAccount*> (po);
		handleNewAccount (pacc->account ());
	}

	void ProtoWrapper::handleNewAccount (Tp::AccountPtr acc)
	{
		if (ProtoName_ != acc->protocolName ())
			return;

		qDebug () << Q_FUNC_INFO << ProtoName_ << acc->nickname () << acc->iconName ();
		auto w = new AccountWrapper (acc, this);
		connect (w,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (w,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
		Accounts_ << w;
		emit accountAdded (w);
	}
}
}
}
