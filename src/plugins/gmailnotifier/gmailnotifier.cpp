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
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "gmailchecker.h"

namespace LeechCraft
{
namespace GmailNotifier
{
	void GmailNotifier::Init (ICoreProxy_ptr)
	{
		NotifierAction_ = 0;

		Util::InstallTranslator ("gmailnotifier");
		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"gmailnotifiersettings.xml");
		XmlSettingsManager::Instance ()->RegisterObject (QList<QByteArray> () << "Login" << "Password",
				this,
				"setAuthorization");

		XmlSettingsManager::Instance ()->RegisterObject ("ShowUnreadNumInTray", this, "setShowUnreadNumInTray");

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

		connect (GmailChecker_,
				SIGNAL (anErrorOccupied (const QString&, const QString&)),
				this,
				SLOT (sendMeNotification (const QString&, const QString&)));
		connect (GmailChecker_,
				SIGNAL (newConversationsAvailable (const QString&, const QString&, int)),
				this,
				SLOT (sendMeNotification (const QString&, const QString&, int)));
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

	QList<QAction*> GmailNotifier::GmailNotifier::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::LCTray && NotifierAction_)
			result << NotifierAction_;
		return result;
	}

	void GmailNotifier::CheckCreateAction ()
	{
		if (NotifierAction_)
			return;

		NotifierAction_ = new QAction (this);
		emit gotActions ({ NotifierAction_ }, ActionsEmbedPlace::LCTray);
	}

	void GmailNotifier::setAuthorization ()
	{
		GmailChecker_->SetAuthSettings (XmlSettingsManager::Instance ()->property ("Login").toString (),
				XmlSettingsManager::Instance ()->property ("Password").toString ());
	}

	void GmailNotifier::setShowUnreadNumInTray ()
	{
		const bool enabled = XmlSettingsManager::Instance ()->property ("ShowUnreadNumInTray").toBool ();
		if (!enabled)
		{
			delete NotifierAction_;
			NotifierAction_ = 0;
		}
		else
			GmailChecker_->checkNow ();
	}

	void GmailNotifier::applyInterval ()
	{
		const int secs = XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt ();
		UpdateTimer_->stop ();
		UpdateTimer_->setInterval (secs * 1000);
		UpdateTimer_->start ();
	}

	void GmailNotifier::sendMeNotification (const QString& title, const QString& msg, int num)
	{
		if (msg == LastMsg_)
			return;

		if (num && XmlSettingsManager::Instance ()->property ("ShowUnreadNumInTray").toBool ())
		{
			CheckCreateAction ();

			NotifierAction_->setText (tr ("%n new message(s)", 0, num));
			NotifierAction_->setToolTip (msg);

			auto font = qApp->font ();
			font.setBold (true);
			font.setItalic (true);

			const QIcon srcIcon (":/gmailnotifier/gmailicon.svg");
			QIcon result;
			for (auto sz : { 16, 22, 32, 48, 64, 96, 128, 192 })
			{
				const auto& px = srcIcon.pixmap (sz, sz);
				result.addPixmap (Util::DrawOverlayText (px, QString::number (num), font, QPen (Qt::black)));
			}
			NotifierAction_->setIcon (result);
		}

		LastMsg_ = msg;
		emit gotEntity (Util::MakeNotification (title, msg, PInfo_));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_gmailnotifier, LeechCraft::GmailNotifier::GmailNotifier);
