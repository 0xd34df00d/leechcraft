/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
{
namespace GmailNotifier
{
	void GmailNotifier::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("gmailnotifier");
		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"gmailnotifiersettings.xml");
		XmlSettingsManager::Instance ().RegisterObject ({ "Login",  "Password"},
				this,
				"setAuthorization");

		GmailChecker_ = new GmailChecker (this);
		setAuthorization ();

		UpdateTimer_ = new QTimer (this);
		applyInterval ();
		UpdateTimer_->start ();

		XmlSettingsManager::Instance ().RegisterObject ("UpdateInterval",
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

		auto manager = new QuarkManager (proxy, this);
		Quark_ = std::make_shared<QuarkComponent> ("gmailnotifier", "GMQuark.qml");
		Quark_->DynamicProps_.append ({ "GMN_proxy", manager });

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
		static QIcon icon ("lcicons:/gmailnotifier/gmailnotifier.svg");
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
		GmailChecker_->SetAuthSettings (XmlSettingsManager::Instance ().property ("Login").toString (),
				XmlSettingsManager::Instance ().property ("Password").toString ());
	}

	void GmailNotifier::applyInterval ()
	{
		const int secs = XmlSettingsManager::Instance ().property ("UpdateInterval").toInt ();
		UpdateTimer_->stop ();
		UpdateTimer_->setInterval (secs * 1000);
		UpdateTimer_->start ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_gmailnotifier, LC::GmailNotifier::GmailNotifier);
