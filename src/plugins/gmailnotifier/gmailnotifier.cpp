/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
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

#include "gmailnotifier.h"
#include <QIcon>
#include <QTimer>
#include <QTranslator>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "gmailchecker.h"

namespace LeechCraft
{
namespace GmailNotifier
{
	void GmailNotifier::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("gmailnotifier"));
		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"gmailnotifiersettings.xml");
		XmlSettingsManager::Instance ()->RegisterObject (QList<QByteArray> () << "Login" << "Password",
				this,
				"setAuthorization");

		GmailChecker_ = new GmailChecker (this);
		setAuthorization ();

		UpdateTimer_ = new QTimer (this);
		UpdateTimer_->setInterval (30000);
		UpdateTimer_->start ();

		connect (UpdateTimer_,
				SIGNAL (timeout ()),
				GmailChecker_,
				SLOT (checkNow ()));

		connect (GmailChecker_,
				SIGNAL (waitMe ()),
				UpdateTimer_,
				SLOT (stop ()));
		connect (GmailChecker_,
				SIGNAL (canContinue ()),
				UpdateTimer_,
				SLOT (start ()));

		connect (GmailChecker_,
				SIGNAL (anErrorOccupied (const QString&, const QString&)),
				this,
				SLOT (sendMeNotification (const QString&, const QString&)));
		connect (GmailChecker_,
				SIGNAL (newConversationsAvailable (const QString&, const QString&)),
				this,
				SLOT (sendMeNotification (const QString&, const QString&)));
	}

	void GmailNotifier::SecondInit ()
	{
	}

	QByteArray GmailNotifier::GetUniqueID () const
	{
		return "org.LeechCraft.GmailNotifier";
	}

	void GmailNotifier::Release ()
	{
	}

	QString GmailNotifier::GetName () const
	{
		return "Gmail Notifier";
	}

	QString GmailNotifier::GetInfo () const
	{
		return tr ("Google mail notification plugin");
	}

	QIcon GmailNotifier::GetIcon () const
	{
		return QIcon (":/gmailnotifier.svg");
	}

	Util::XmlSettingsDialog_ptr GmailNotifier::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void GmailNotifier::setAuthorization ()
	{
		GmailChecker_->SetAuthSettings (XmlSettingsManager::Instance ()->property ("Login").toString (),
				XmlSettingsManager::Instance ()->property ("Password").toString ());
	}

	void GmailNotifier::sendMeNotification (const QString& title, const QString& msg)
	{
		if (msg == LastMsg_)
			return;

		LastMsg_ = msg;
		Entity e = Util::MakeNotification (title, msg, PInfo_);
		emit gotEntity (e);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_gmailnotifier, LeechCraft::GmailNotifier::GmailNotifier);
