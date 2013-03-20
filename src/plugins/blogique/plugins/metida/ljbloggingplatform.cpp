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

#include "ljbloggingplatform.h"
#include <QIcon>
#include <QInputDialog>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QMainWindow>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/passutils.h>
#include "core.h"
#include "ljaccount.h"
#include "ljaccountconfigurationwidget.h"
#include "postoptionswidget.h"
#include "localstorage.h"
#include "recentcommentssidewidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJBloggingPlatform::LJBloggingPlatform (QObject *parent)
	: QObject (parent)
	, ParentBlogginPlatfromPlugin_ (parent)
	, PluginProxy_ (0)
	, LJUser_ (new QAction (Core::Instance ().GetCoreProxy ()->GetIcon ("user-properties"),
			tr ("Add LJ user"), this))
	, LJCut_ (new QAction (Core::Instance ().GetCoreProxy ()->GetIcon ("view-split-top-bottom"),
			"Cut", this))
	, FirstSeparator_ (new QAction (this))
	, MessageCheckingTimer_ (new QTimer (this))
	, CommentsCheckingTimer_ (new QTimer (this))
	{
		FirstSeparator_->setSeparator (true);

		connect (LJUser_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddLJUser ()));
		connect (LJUser_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddLJCut ()));

		connect (MessageCheckingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkForMessages ()));
		connect (CommentsCheckingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkForComments ()));

		XmlSettingsManager::Instance ().RegisterObject ("CheckingInboxEnabled",
				this, "handleMessageChecking");
		XmlSettingsManager::Instance ().RegisterObject ("CheckingCommentsEnabled",
				this, "handleCommentsChecking");
		handleMessageChecking ();
		handleCommentsChecking ();
	}

	QObject* LJBloggingPlatform::GetQObject ()
	{
		return this;
	}

	IBloggingPlatform::BloggingPlatfromFeatures LJBloggingPlatform::GetFeatures () const
	{
		return BPFSupportsProfiles | BPFSelectablePostDestination | BPFSupportsBackup;
	}

	QObjectList LJBloggingPlatform::GetRegisteredAccounts ()
	{
		QObjectList result;
		Q_FOREACH (auto acc, LJAccounts_)
			result << acc;
		return result;
	}

	QObject* LJBloggingPlatform::GetParentBloggingPlatformPlugin () const
	{
		return ParentBlogginPlatfromPlugin_;
	}

	QString LJBloggingPlatform::GetBloggingPlatformName () const
	{
		return "LiveJournal";
	}

	QIcon LJBloggingPlatform::GetBloggingPlatformIcon () const
	{
		return QIcon (":/plugins/blogique/plugins/metida/resources/images/livejournalicon.svg");
	}

	QByteArray LJBloggingPlatform::GetBloggingPlatformID () const
	{
		return "Blogique.Metida.LiveJournal";
	}

	QList<QWidget*> LJBloggingPlatform::GetAccountRegistrationWidgets (IBloggingPlatform::AccountAddOptions)
	{
		QList<QWidget*> result;
		result << new LJAccountConfigurationWidget ();
		return result;
	}

	void LJBloggingPlatform::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		auto w = qobject_cast<LJAccountConfigurationWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		LJAccount *account = new LJAccount (name, this);
		account->FillSettings (w);

		const QString& pass = w->GetPassword ();
		if (!pass.isEmpty ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + account->GetAccountID (),
					&Core::Instance ());

		LJAccounts_ << account;
		saveAccounts ();
		emit accountAdded (account);
		account->Init ();
		Core::Instance ().GetLocalStorage ()->AddAccount (account->GetAccountID ());
	}

	void LJBloggingPlatform::RemoveAccount (QObject *account)
	{
		LJAccount *acc = qobject_cast<LJAccount*> (account);
		if (LJAccounts_.removeAll (acc))
		{
			emit accountRemoved (account);
			Core::Instance ().GetLocalStorage ()->RemoveAccount (acc->GetAccountID ());
			account->deleteLater ();
			saveAccounts ();
		}
	}

	QList<QAction*> LJBloggingPlatform::GetEditorActions () const
	{
		return { FirstSeparator_, LJUser_, LJCut_ };
	}

	QList<QWidget*> LJBloggingPlatform::GetBlogiqueSideWidgets () const
	{
		return { new PostOptionsWidget, new RecentCommentsSideWidget };
	}

	void LJBloggingPlatform::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void LJBloggingPlatform::Prepare ()
	{
		RestoreAccounts ();
	}

	void LJBloggingPlatform::Release ()
	{
		saveAccounts ();
	}

	void LJBloggingPlatform::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
						"_Blogique_Metida_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings.value ("SerializedData").toByteArray ();
			LJAccount *acc = LJAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			LJAccounts_ << acc;
			emit accountAdded (acc);

			acc->Init ();
			Core::Instance ().GetLocalStorage ()->AddAccount (acc->GetAccountID ());
		}
		settings.endArray ();
	}

	void LJBloggingPlatform::saveAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
						"_Blogique_Metida_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0, size = LJAccounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					LJAccounts_.at (i)->Serialize ());
		}
		settings.endArray ();
		settings.sync ();
	}

	void LJBloggingPlatform::handleAddLJUser ()
	{
		auto rootWM = Core::Instance ().GetCoreProxy ()->GetRootWindowsManager ();
		QString name = QInputDialog::getText (rootWM->GetPreferredWindow (),
				tr ("Add LJ User"),
				tr ("Enter LJ user name"));
		if (name.isEmpty ())
			return;
	}

	void LJBloggingPlatform::handleAddLJCut ()
	{

	}

	void LJBloggingPlatform::handleAccountValidated (bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an IAccount";;
			return;
		}

		emit accountValidated (acc->GetQObject (), validated);
	}

	void LJBloggingPlatform::handleMessageChecking ()
	{
		if (XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool ())
			MessageCheckingTimer_->start (XmlSettingsManager::Instance ()
					.property ("UpdateInboxInterval").toInt () * 1000);
		else if (MessageCheckingTimer_->isActive ())
			MessageCheckingTimer_->stop ();
	}

	void LJBloggingPlatform::handleCommentsChecking ()
	{
		if (XmlSettingsManager::Instance ().Property ("CheckingCommentsEnabled", true).toBool ())
			CommentsCheckingTimer_->start (XmlSettingsManager::Instance ()
					.property ("UpdateCommentsInterval").toInt () * 60 * 1000);
		else if (CommentsCheckingTimer_->isActive ())
			CommentsCheckingTimer_->stop ();
	}

	void LJBloggingPlatform::checkForMessages ()
	{
		for (auto account : LJAccounts_)
			account->RequestInbox ();
	}

	void LJBloggingPlatform::checkForComments ()
	{
		for (auto account : LJAccounts_)
			account->RequestRecentComments ();
	}

}
}
}
