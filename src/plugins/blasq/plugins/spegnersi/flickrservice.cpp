/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flickrservice.h"
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include "flickraccount.h"

namespace LC
{
namespace Blasq
{
namespace Spegnersi
{
	FlickrService::FlickrService (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Spegnersi");
		settings.beginGroup ("Accounts");
		for (const auto& key : settings.childKeys ())
		{
			const auto& serialized = settings.value (key).toByteArray ();
			auto acc = FlickrAccount::Deserialize (serialized, this, proxy);
			if (acc)
				AddAccount (acc);
		}
		settings.endGroup ();
	}

	QObject* FlickrService::GetQObject ()
	{
		return this;
	}

	QString FlickrService::GetServiceName () const
	{
		return "Flickr";
	}

	QIcon FlickrService::GetServiceIcon () const
	{
		static QIcon icon (":/blasq/spegnersi/resources/images/flickricon.png");
		return icon;
	}

	QList<IAccount*> FlickrService::GetRegisteredAccounts () const
	{
		QList<IAccount*> result;
		std::copy (Accounts_.begin (), Accounts_.end (), std::back_inserter (result));
		return result;
	}

	QList<QWidget*> FlickrService::GetAccountRegistrationWidgets () const
	{
		return {};
	}

	void FlickrService::RegisterAccount (const QString& name, const QList<QWidget*>&)
	{
		auto acc = new FlickrAccount (name, this, Proxy_);
		AddAccount (acc);
		saveAccount (acc);
	}

	void FlickrService::RemoveAccount (IAccount *acc)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Spegnersi");
		settings.beginGroup ("Accounts");
		settings.remove (acc->GetID ());
		settings.endGroup ();

		const auto pos = std::find (Accounts_.begin (), Accounts_.end (), acc);
		if (pos == Accounts_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetID ()
					<< "not found";
			return;
		}

		emit accountRemoved (*pos);
		(*pos)->deleteLater ();

		Accounts_.erase (pos);
	}

	void FlickrService::AddAccount (FlickrAccount *acc)
	{
		Accounts_ << acc;
		emit accountAdded (acc);
		connect (acc,
				SIGNAL (accountChanged (FlickrAccount*)),
				this,
				SLOT (saveAccount (FlickrAccount*)));
	}

	void FlickrService::saveAccount (FlickrAccount *acc)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Blasq_Spegnersi");
		settings.beginGroup ("Accounts");
		settings.setValue (acc->GetID (), acc->Serialize ());
		settings.endGroup ();
	}
}
}
}
