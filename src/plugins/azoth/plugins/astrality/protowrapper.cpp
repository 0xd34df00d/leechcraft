/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "protowrapper.h"
#include <QIcon>
#include <AccountManager>
#include <PendingReady>
#include <PendingAccount>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/prelude.h>
#include <util/sll/functional.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include "accountregfirstpage.h"
#include "accountwrapper.h"

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	ProtoWrapper::ProtoWrapper (Tp::ConnectionManagerPtr cm,
			const QString& protoName, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, CM_ (cm)
	, ProtoName_ (protoName)
	, Proxy_ (proxy)
	, ProtoInfo_ (CM_->protocol (ProtoName_))
	{
		const auto& sb = QDBusConnection::sessionBus ();
		auto accf = Tp::AccountFactory::create (sb,
				Tp::Account::FeatureCore |
				Tp::Account::FeatureAvatar |
				Tp::Account::FeatureProtocolInfo |
				Tp::Account::FeatureCapabilities |
				Tp::Account::FeatureProfile);
		auto channelf = Tp::ChannelFactory::create (sb);
		auto connf = Tp::ConnectionFactory::create (sb,
				Tp::Connection::FeatureConnected |
				Tp::Connection::FeatureRoster |
				Tp::Connection::FeatureSimplePresence |
				Tp::Connection::FeatureRosterGroups);
		auto contactf = Tp::ContactFactory::create (Tp::Contact::FeatureAlias |
				Tp::Contact::FeatureSimplePresence |
				Tp::Contact::FeatureAvatarData |
				Tp::Contact::FeatureAvatarToken |
				Tp::Contact::FeatureCapabilities |
				Tp::Contact::FeatureLocation |
				Tp::Contact::FeatureInfo);
		AM_ = Tp::AccountManager::create (accf, connf, channelf, contactf);

		connect (AM_->becomeReady (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleAMReady (Tp::PendingOperation*)));
		connect (AM_.data (),
				SIGNAL (newAccount (Tp::AccountPtr)),
				this,
				SLOT (handleNewAccount (Tp::AccountPtr)));
	}

	void ProtoWrapper::Release ()
	{
		for (const auto accObj : GetRegisteredAccounts ())
		{
			auto acc = qobject_cast<AccountWrapper*> (accObj);
			emit accountRemoved (acc);
			acc->Shutdown ();
		}
	}

	QVariantMap ProtoWrapper::GetParamsFromWidgets (const QList<QWidget*>& widgets) const
	{
		QVariantMap params;
		auto fpg = qobject_cast<AccountRegFirstPage*> (widgets.value (0));
		auto chSet = [&params, this] (QString param, QVariant value)
		{
			if (ProtoInfo_.hasParameter (param))
				params [param] = value;
		};
		if (!fpg->GetAccountID ().isEmpty ())
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

		return params;
	}

	AccountWrapper::Settings ProtoWrapper::GetSettingsFromWidgets (const QList<QWidget*>& widgets) const
	{
		auto fpg = qobject_cast<AccountRegFirstPage*> (widgets.value (0));
		AccountWrapper::Settings s;
		fpg->Augment (s);
		return s;
	}

	QObject* ProtoWrapper::GetQObject ()
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
		return Util::Map (Accounts_, Util::Upcast<QObject*>);
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
		QIcon result = QIcon::fromTheme (ProtoInfo_.iconName ());
		if (result.isNull ())
			result = Proxy_->GetIconThemeManager ()->GetPluginIcon ();
		return result;
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

		const QVariantMap& params = GetParamsFromWidgets (widgets);

		auto pending = AM_->createAccount (CM_->name (), ProtoName_, name, params, QVariantMap ());
		connect (pending,
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleAccountCreated (Tp::PendingOperation*)));
		PendingSettings_ [pending] = GetSettingsFromWidgets (widgets);
	}

	void ProtoWrapper::RemoveAccount (QObject *accObj)
	{
		auto acc = qobject_cast<AccountWrapper*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "not an AccountWrapper"
					<< accObj;
			return;
		}

		acc->RemoveThis ();
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

		for (auto acc : AM_->allAccounts ())
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
						Priority::Critical));
			return;
		}

		auto pacc = qobject_cast<Tp::PendingAccount*> (po);
		auto acc = handleNewAccount (pacc->account ());

		if (acc && PendingSettings_.contains (pacc))
			acc->SetSettings (PendingSettings_.take (pacc));
	}

	AccountWrapper* ProtoWrapper::handleNewAccount (Tp::AccountPtr acc)
	{
		if (ProtoName_ != acc->protocolName ())
			return 0;

		for (auto w : Accounts_)
			if (w->GetOurID () == acc->uniqueIdentifier ())
				return w;

		qDebug () << Q_FUNC_INFO << ProtoName_ << acc->nickname () << acc->iconName ();
		auto w = new AccountWrapper (acc, Proxy_, this);
		connect (w,
				SIGNAL (gotEntity (LC::Entity)),
				this,
				SIGNAL (gotEntity (LC::Entity)));
		connect (w,
				SIGNAL (removeFinished (AccountWrapper*)),
				this,
				SLOT (handleAccountRemoved (AccountWrapper*)));
		Accounts_ << w;
		emit accountAdded (w);

		return w;
	}

	void ProtoWrapper::handleAccountRemoved (AccountWrapper *aw)
	{
		Accounts_.removeAll (aw);
		emit accountRemoved (aw);
		aw->deleteLater ();
	}
}
}
}
