/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "localbloggingplatform.h"
#include <QIcon>
#include <QInputDialog>
#include <QSettings>
#include <QtDebug>
#include <QMainWindow>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/passutils.h>
#include <util/util.h>
#include "accountconfigurationwidget.h"
#include "core.h"
#include "localblogaccount.h"
#include "postoptionswidget.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	LocalBloggingPlatform::LocalBloggingPlatform (QObject *parent)
	: QObject (parent)
	, ParentBlogginPlatfromPlugin_ (parent)
	, PluginProxy_ (0)
	{
	}

	QObject* LocalBloggingPlatform::GetQObject ()
	{
		return this;
	}

	IBloggingPlatform::BloggingPlatfromFeatures LocalBloggingPlatform::GetFeatures () const
	{
		return BPFSupportsRegistration;
	}

	QObjectList LocalBloggingPlatform::GetRegisteredAccounts ()
	{
		QObjectList result;
		for (auto acc : Accounts_)
				result << acc;
		return result;
	}

	QObject* LocalBloggingPlatform::GetParentBloggingPlatformPlugin () const
	{
		return ParentBlogginPlatfromPlugin_;
	}

	QString LocalBloggingPlatform::GetBloggingPlatformName () const
	{
		return "Local blog";
	}

	QIcon LocalBloggingPlatform::GetBloggingPlatformIcon () const
	{
		return QIcon ();
	}

	QByteArray LocalBloggingPlatform::GetBloggingPlatformID () const
	{
		return "Blogique.Hestia.LocalBlog";
	}

	QList<QWidget*> LocalBloggingPlatform::GetAccountRegistrationWidgets (IBloggingPlatform::AccountAddOptions opts)
	{
		return { new AccountConfigurationWidget (0, opts) };
	}

	void LocalBloggingPlatform::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		auto w = qobject_cast<AccountConfigurationWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		LocalBlogAccount *account = new LocalBlogAccount (name, this);
		account->FillSettings (w);

		const QString& path = w->GetAccountBasePath ();
		if (!path.isEmpty ())
		{
			Accounts_ << account;
			saveAccounts ();
			emit accountAdded (account);
			account->Init ();
		}
	}

	void LocalBloggingPlatform::RemoveAccount (QObject *account)
	{
		auto acc = qobject_cast<LocalBlogAccount*> (account);
		if (Accounts_.removeAll (acc))
		{
			emit accountRemoved (account);
			account->deleteLater ();
			saveAccounts ();
		}
	}

	QList<QAction*> LocalBloggingPlatform::GetEditorActions () const
	{
		return QList<QAction*> ();
	}

	QList<QWidget*> LocalBloggingPlatform::GetBlogiqueSideWidgets () const
	{
		return { new PostOptionsWidget };
	}

	void LocalBloggingPlatform::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void LocalBloggingPlatform::Prepare ()
	{
		RestoreAccounts ();
	}

	void LocalBloggingPlatform::Release ()
	{
		saveAccounts ();
	}

	void LocalBloggingPlatform::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Blogique_Hestia_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings.value ("SerializedData").toByteArray ();
			LocalBlogAccount *acc = LocalBlogAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			Accounts_ << acc;
			if (!acc->IsValid ())
				Core::Instance ().SendEntity (Util::MakeNotification ("Blogique",
						tr ("You have invalid account data."),
						PWarning_));

			emit accountAdded (acc);
			acc->Init ();
		}
		settings.endArray ();
	}

	void LocalBloggingPlatform::saveAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Blogique_Hestia_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0, size = Accounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					Accounts_.at (i)->Serialize ());
		}
		settings.endArray ();
	}

	void LocalBloggingPlatform::handleAccountValidated (bool valid)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an IAccount";;
			return;
		}

		emit accountValidated (acc->GetQObject (), valid);
	}

}
}
}
