/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localbloggingplatform.h"
#include <QIcon>
#include <QInputDialog>
#include <QSettings>
#include <QtDebug>
#include <QMainWindow>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/xpc/passutils.h>
#include <util/xpc/util.h>
#include "accountconfigurationwidget.h"
#include "localblogaccount.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	LocalBloggingPlatform::LocalBloggingPlatform (QObject *parent)
	: ParentBlogginPlatfromPlugin_ { parent }
	{
	}

	QObject* LocalBloggingPlatform::GetQObject ()
	{
		return this;
	}

	IBloggingPlatform::BloggingPlatfromFeatures LocalBloggingPlatform::GetFeatures () const
	{
		return BPFSupportsRegistration | BPFLocalBlog;
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

	QList<QWidget*> LocalBloggingPlatform::GetAccountRegistrationWidgets (AccountAddOptions opts,
			const QString& accName)
	{
		return { new AccountConfigurationWidget (0, opts, accName) };
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

		auto account = new LocalBlogAccount (name, this);
		account->FillSettings (w);

		const QString& path = w->GetAccountBasePath ();
		if (!path.isEmpty ())
		{
			Accounts_ << account;
			saveAccounts ();
			HandleAccountObject (account);
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
		return {};
	}

	QList<InlineTagInserter> LocalBloggingPlatform::GetInlineTagInserters () const
	{
		return {};
	}

	QList<QWidget*> LocalBloggingPlatform::GetBlogiqueSideWidgets () const
	{
		return {};
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

	IAdvancedHTMLEditor::CustomTags_t LocalBloggingPlatform::GetCustomTags () const
	{
		return {};
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
			auto acc = LocalBlogAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			Accounts_ << acc;
			if (!acc->IsValid ())
				qWarning () << Q_FUNC_INFO
						<< "account is invalid";

			HandleAccountObject (acc);
		}
		settings.endArray ();
	}

	void LocalBloggingPlatform::HandleAccountObject (LocalBlogAccount *account)
	{
		emit accountAdded (account);

		connect (account,
				&LocalBlogAccount::accountValidated,
				this,
				[this, account] (bool valid) { emit accountValidated (account->GetQObject (), valid); });
		connect (account,
				&LocalBlogAccount::accountSettingsChanged,
				this,
				&LocalBloggingPlatform::saveAccounts);

		account->Init ();
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
}
}
}
