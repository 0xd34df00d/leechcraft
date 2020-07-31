/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "picasaservice.h"
#include <QIcon>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include "authmanager.h"
#include "picasaaccount.h"
#include "registerpage.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	PicasaService::PicasaService (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		AuthManager_ = new AuthManager (proxy, this);
		connect (AuthManager_,
				SIGNAL (authSuccess (QObject*)),
				this,
				SLOT (handleAuthSuccess (QObject*)));

		ReadAccounts ();
	}

	QObject* PicasaService::GetQObject ()
	{
		return this;
	}

	QString PicasaService::GetServiceName () const
	{
		return "Picasa";
	}

	QIcon PicasaService::GetServiceIcon () const
	{
		static QIcon icon (":/blasq/vangog/resources/images/picasalogo.png");
		return icon;
	}

	void PicasaService::Release ()
	{
		for (auto acc : Accounts_)
			acc->Release ();
	}

	QList<IAccount*> PicasaService::GetRegisteredAccounts () const
	{
		QList<IAccount*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QList<QWidget*> PicasaService::GetAccountRegistrationWidgets () const
	{
		return { new RegisterPage };
	}

	void PicasaService::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		if (!widgets.isEmpty ())
		{
			const auto& login = qobject_cast<RegisterPage*> (widgets.at (0))->GetLogin ();
			auto acc = new PicasaAccount (name, this, Proxy_, login);
			AuthManager_->Auth (acc);
		}
	}

	void PicasaService::RemoveAccount (IAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Vangog");
		settings.beginGroup ("Accounts");
		settings.remove (account->GetID ());
		settings.endGroup ();

		const auto pos = std::find (Accounts_.begin (), Accounts_.end (), account);
		if (pos == Accounts_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< account->GetID ()
					<< "not found";
			return;
		}

		emit accountRemoved (*pos);
		(*pos)->deleteLater ();

		Accounts_.erase (pos);
	}

	void PicasaService::AddAccount (PicasaAccount *account)
	{
		Accounts_ << account;
		emit accountAdded (account);
		connect (account,
				SIGNAL (accountChanged (PicasaAccount*)),
				this,
				SLOT (saveAccount (PicasaAccount*)));
	}

	void PicasaService::ReadAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Vangog");
		settings.beginGroup ("Accounts");
		for (const auto& key : settings.childKeys ())
		{
			const auto& serialized = settings.value (key).toByteArray ();
			if (auto acc = PicasaAccount::Deserialize (serialized, this, Proxy_))
				AddAccount (acc);
		}
		settings.endGroup ();
	}

	void PicasaService::saveAccount (PicasaAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Vangog");
		settings.beginGroup ("Accounts");
		settings.setValue (account->GetID (), account->Serialize ());
		settings.endGroup ();
	}

	void PicasaService::handleAuthSuccess (QObject *accObj)
	{
		auto acc (qobject_cast<PicasaAccount*> (accObj));
		AddAccount (acc);
		saveAccount (acc);
	}

}
}
}
