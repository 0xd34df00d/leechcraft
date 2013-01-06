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
#include <QAction>
#include <QApplication>
#include <QPen>
#include <QTranslator>
#include <util/util.h>
#include <util/sys/paths.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "gmailchecker.h"
#include "notifier.h"
#include "quarkmanager.h"

namespace LeechCraft
{
namespace GmailNotifier
{
	void GmailNotifier::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("gmailnotifier");
		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"gmailnotifiersettings.xml");
		XmlSettingsManager::Instance ()->RegisterObject ({ "Login",  "Password"},
				this,
				"setAuthorization");

		GmailChecker_ = new GmailChecker (this);
		setAuthorization ();

		UpdateTimer_ = new QTimer (this);
		applyInterval ();
		UpdateTimer_->start ();

		XmlSettingsManager::Instance ()->RegisterObject ("UpdateInterval",
				this,
				"applyInterval");

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

		Notifier_ = new Notifier (proxy, this);
		connect (GmailChecker_,
				SIGNAL (gotConversations (ConvInfos_t)),
				Notifier_,
				SLOT (notifyAbout (ConvInfos_t)));

		auto manager = new QuarkManager (this);
		const auto& quarkPath = Util::GetSysPath (Util::SysPath::QML,
				"gmailnotifier", "GMQuark.qml");
		Quark_.Url_ = QUrl::fromLocalFile (quarkPath);
		Quark_.DynamicProps_ << QPair<QString, QObject*> ("GMN_proxy", manager);

		connect (GmailChecker_,
				SIGNAL (gotConversations (ConvInfos_t)),
				manager,
				SLOT (handleConversations (ConvInfos_t)));
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
		static QIcon icon (":/gmailnotifier/gmailnotifier.svg");
		return icon;
	}

	Util::XmlSettingsDialog_ptr GmailNotifier::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QuarkComponents_t GmailNotifier::GmailNotifier::GetComponents () const
	{
		return { Quark_ };
	}

	void GmailNotifier::setAuthorization ()
	{
		GmailChecker_->SetAuthSettings (XmlSettingsManager::Instance ()->property ("Login").toString (),
				XmlSettingsManager::Instance ()->property ("Password").toString ());
	}

	void GmailNotifier::applyInterval ()
	{
		const int secs = XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt ();
		UpdateTimer_->stop ();
		UpdateTimer_->setInterval (secs * 1000);
		UpdateTimer_->start ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_gmailnotifier, LeechCraft::GmailNotifier::GmailNotifier);
