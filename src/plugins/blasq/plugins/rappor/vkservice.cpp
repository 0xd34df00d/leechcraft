/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkservice.h"
#include <QIcon>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include "vkaccount.h"

namespace LC
{
namespace Blasq
{
namespace Rappor
{
	VkService::VkService (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Rappor");
		settings.beginGroup ("Accounts");
		for (const auto& key : settings.childKeys ())
		{
			const auto& serialized = settings.value (key).toByteArray ();
			if (auto acc = VkAccount::Deserialize (serialized, this, proxy))
				AddAccount (acc);
		}
		settings.endGroup ();
	}

	QObject* VkService::GetQObject ()
	{
		return this;
	}

	QString VkService::GetServiceName () const
	{
		return tr ("VKontakte");
	}

	QIcon VkService::GetServiceIcon () const
	{
		static QIcon icon (":/blasq/rappor/resources/images/vk.png");
		return icon;
	}

	QList<IAccount*> VkService::GetRegisteredAccounts () const
	{
		QList<IAccount*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QList<QWidget*> VkService::GetAccountRegistrationWidgets () const
	{
		return {};
	}

	void VkService::RegisterAccount (const QString& name, const QList<QWidget*>&)
	{
		auto acc = new VkAccount (name, this, Proxy_);
		AddAccount (acc);
		saveAccount (acc);
	}

	void VkService::RemoveAccount (IAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Rappor");
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

	void VkService::AddAccount (VkAccount *account)
	{
		Accounts_ << account;
		emit accountAdded (account);
		connect (account,
				SIGNAL (accountChanged (VkAccount*)),
				this,
				SLOT (saveAccount (VkAccount*)));
	}

	void VkService::saveAccount (VkAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Rappor");
		settings.beginGroup ("Accounts");
		settings.setValue (account->GetID (), account->Serialize ());
		settings.endGroup ();
	}
}
}
}
