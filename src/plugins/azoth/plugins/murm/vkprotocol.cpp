/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkprotocol.h"
#include <QIcon>
#include <QApplication>
#include <QSettings>
#include <util/sys/resourceloader.h>
#include "vkaccount.h"
#include "mucjoinwidget.h"
#include "photourlstorage.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VkProtocol::VkProtocol (ICoreProxy_ptr proxy, IProxyObject *azothProxy, QObject *plugin)
	: QObject (plugin)
	, Proxy_ (proxy)
	, AzothProxy_ (azothProxy)
	, Plugin_ (plugin)
	, PhotoUrlStorage_ (new PhotoUrlStorage (this))
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Murm");
		settings.beginGroup ("Accounts");
		for (const auto& id : settings.childKeys ())
		{
			const auto& data = settings.value (id).toByteArray ();
			if (auto acc = VkAccount::Deserialize (data, this, proxy))
				AddAccount (acc);
		}
		settings.endGroup ();
	}

	VkProtocol::~VkProtocol ()
	{
		for (auto acc : Accounts_)
			emit accountRemoved (acc);
	}

	IProxyObject* VkProtocol::GetAzothProxy () const
	{
		return AzothProxy_;
	}

	QObject* VkProtocol::GetQObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures VkProtocol::GetFeatures () const
	{
		return PFSupportsMUCs | PFMUCsJoinable;
	}

	QList<QObject*> VkProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QObject* VkProtocol::GetParentProtocolPlugin () const
	{
		return Plugin_;
	}

	QString VkProtocol::GetProtocolName () const
	{
		return tr ("VKontakte");
	}

	QIcon VkProtocol::GetProtocolIcon () const
	{
		static QIcon icon = []
		{
			static Util::ResourceLoader loader { "azoth/murm/clients" };
			loader.AddGlobalPrefix ();
			loader.AddLocalPrefix ();
			return loader.LoadPixmap ("vk");
		} ();
		return icon;
	}

	QByteArray VkProtocol::GetProtocolID () const
	{
		return "Murm.VK";
	}

	QList<QWidget*> VkProtocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
	{
		return {};
	}

	void VkProtocol::RegisterAccount (const QString& name, const QList<QWidget*>&)
	{
		auto acc = new VkAccount (name, this, Proxy_, {}, {});
		acc->Init ();
		saveAccount (acc);
		AddAccount (acc);
	}

	QWidget* VkProtocol::GetMUCJoinWidget ()
	{
		return new MucJoinWidget (Proxy_);
	}

	void VkProtocol::RemoveAccount (QObject *accObj)
	{
		auto acc = qobject_cast<VkAccount*> (accObj);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Murm");
		settings.beginGroup ("Accounts");
		settings.remove (acc->GetAccountID ());
		settings.endGroup ();

		emit accountRemoved (accObj);

		Accounts_.removeAll (acc);

		acc->deleteLater ();
	}

	void VkProtocol::AddAccount (VkAccount *acc)
	{
		Accounts_ << acc;
		emit accountAdded (acc);

		connect (acc,
				SIGNAL (accountChanged (VkAccount*)),
				this,
				SLOT (saveAccount (VkAccount*)));
	}

	void VkProtocol::saveAccount (VkAccount *account)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Murm");
		settings.beginGroup ("Accounts");
		settings.setValue (account->GetAccountID (), account->Serialize ());
		settings.endGroup ();
	}

	PhotoUrlStorage* VkProtocol::GetPhotoUrlStorage () const
	{
		return PhotoUrlStorage_;
	}
}
}
}
