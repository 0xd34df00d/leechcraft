/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "inbandaccountregsecondpage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDomElement>
#include <QLineEdit>
#include <QXmppBobManager.h>
#include "inbandaccountregfirstpage.h"
#include "util.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsRegister = "jabber:iq:register";

	InBandAccountRegSecondPage::InBandAccountRegSecondPage (InBandAccountRegFirstPage *first, QWidget *parent)
	: QWizardPage (parent)
	, Client_ (new QXmppClient (this))
	, BobManager_ (new QXmppBobManager)
	, FirstPage_ (first)
	, FB_ (FormBuilder (QString (), BobManager_))
	, Widget_ (0)
	, State_ (SIdle)
	{
		Q_FOREACH (QXmppClientExtension *ext, Client_->extensions ())
			Client_->removeExtension (ext);

		Client_->addExtension (BobManager_);

		setLayout (new QVBoxLayout);

		connect (Client_,
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));
		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleError (QXmppClient::Error)));
		connect (Client_,
				SIGNAL (iqReceived (const QXmppIq&)),
				this,
				SLOT (handleIqReceived (const QXmppIq&)));
	}

	void InBandAccountRegSecondPage::Register ()
	{
		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsRegister);

		switch (FormType_)
		{
		case FTLegacy:
			Q_FOREACH (const QXmppElement& elem, LFB_.GetFilledChildren ())
				queryElem.appendChild (elem);
			break;
		case FTNew:
		{
			QByteArray arr;
			{
				QXmlStreamWriter w (&arr);
				FB_.GetForm ().toXml (&w);
			}
			QDomDocument dom;
			dom.setContent (arr);
			queryElem.appendChild (QXmppElement (dom.documentElement ()));
			break;
		}
		}

		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions (queryElem);
		Client_->sendPacket (iq);

		SetState (SAwaitingRegistrationResult);
	}

	QString InBandAccountRegSecondPage::GetJID () const
	{
		if (FormType_ != FTLegacy)
			return QString ();
		return LFB_.GetUsername () + '@' + FirstPage_->GetServerName ();
	}

	QString InBandAccountRegSecondPage::GetPassword () const
	{
		if (FormType_ != FTLegacy)
			return QString ();
		return LFB_.GetPassword ();
	}

	bool InBandAccountRegSecondPage::isComplete () const
	{
		switch (State_)
		{
		case SError:
		case SIdle:
		case SConnecting:
		case SFetchingForm:
		case SAwaitingRegistrationResult:
			return false;
		case SAwaitingUserInput:
			if (!Widget_)
				return false;
			Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ())
				if (edit->text ().isEmpty ())
					return false;
			return true;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown state"
					<< State_;
			return false;
		}
	}

	void InBandAccountRegSecondPage::initializePage ()
	{
		QWizardPage::initializePage ();

		const QString& server = FirstPage_->GetServerName ();
		ShowMessage (tr ("Connecting to %1...").arg (server));

		if (Client_->isConnected ())
			Client_->disconnectFromServer ();

		QXmppConfiguration conf;
		conf.setDomain (server);
		conf.setSASLAuthMechanism (QXmppConfiguration::SASLAnonymous);
		conf.setIgnoreAuth (true);
		Client_->connectToServer (conf);

		SetState (SConnecting);
	}

	void InBandAccountRegSecondPage::ShowMessage (const QString& msg)
	{
		qDeleteAll (findChildren<QWidget*> ());

		layout ()->addWidget (new QLabel (msg));
	}

	void InBandAccountRegSecondPage::SetState (InBandAccountRegSecondPage::State state)
	{
		State_ = state;
		emit completeChanged ();
	}

	void InBandAccountRegSecondPage::HandleRegForm (const QXmppIq& iq)
	{
		QXmppElement queryElem;
		Q_FOREACH (const QXmppElement& elem, iq.extensions ())
		{
			if (elem.tagName () == "query" &&
					elem.attribute ("xmlns") == NsRegister)
			{
				queryElem = elem;
				break;
			}
		}

		if (queryElem.isNull ())
		{
			SetState (SError);
			ShowMessage (tr ("Service unavailable"));
			return;
		}

		qDeleteAll (findChildren<QWidget*> ());

		const QXmppElement& formElem = queryElem.firstChildElement ("x");
		if ((formElem.attribute ("xmlns") == NsRegister ||
					formElem.attribute ("xmlns") == "jabber:x:data") &&
				formElem.attribute ("type") == "form")
		{
			QXmppDataForm form;
			form.parse (XooxUtil::XmppElem2DomElem (formElem));
			Widget_ = FB_.CreateForm (form);
			FormType_ = FTNew;
		}
		else
		{
			Widget_ = LFB_.CreateForm (queryElem);
			FormType_ = FTLegacy;
		}

		if (!Widget_)
		{
			SetState (SError);
			qWarning () << Q_FUNC_INFO
					<< "got null widget";
			return;
		}

		layout ()->addWidget (Widget_);
		Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ())
			connect (edit,
					SIGNAL (textChanged (const QString&)),
					this,
					SIGNAL (completeChanged ()));

		SetState (SAwaitingUserInput);
	}

	void InBandAccountRegSecondPage::HandleRegResult (const QXmppIq& iq)
	{
		if (iq.type () == QXmppIq::Result)
			emit successfulReg ();
		else if (iq.type () != QXmppIq::Error)
		{
			qWarning () << Q_FUNC_INFO
					<< "strange iq type"
					<< iq.type ();
			return;
		}

		QString msg;
		Q_FOREACH (const QXmppElement& elem, iq.extensions ())
		{
			if (elem.tagName () != "error")
				continue;

			if (!elem.firstChildElement ("conflict").isNull ())
				msg = tr ("data conflict");
			else if (!elem.firstChildElement ("not-acceptable").isNull ())
				msg = tr ("data is not acceptable");
			else
				msg = tr ("general error:") +
					' ' + elem.firstChildElement ().tagName ();
		}
		if (msg.isEmpty ())
			msg = tr ("general registration error");
		emit regError (msg);
	}

	void InBandAccountRegSecondPage::handleConnected ()
	{
		ShowMessage ("Fetching registration form...");

		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsRegister);

		QXmppIq iq;
		iq.setExtensions (queryElem);
		Client_->sendPacket (iq);

		SetState (SFetchingForm);
	}

	void InBandAccountRegSecondPage::handleError (QXmppClient::Error error)
	{
		QString msg;
		switch (error)
		{
		case QXmppClient::SocketError:
			msg = tr ("Socket error:") + ' ' + QString::number (Client_->socketError ()) + '.';
			break;
		case QXmppClient::KeepAliveError:
			msg = tr ("Keep alive error.");
			break;
		case QXmppClient::XmppStreamError:
			msg = tr ("XMPP error:") + ' ' + QString::number (Client_->xmppStreamError ()) + '.';
			break;
		case QXmppClient::NoError:
			msg = tr ("No error.");
			break;
		}

		ShowMessage (msg);

		SetState (SError);
	}

	void InBandAccountRegSecondPage::handleIqReceived (const QXmppIq& iq)
	{
		switch (State_)
		{
		case SFetchingForm:
			HandleRegForm (iq);
			break;
		case SAwaitingRegistrationResult:
			HandleRegResult (iq);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "wrong state for incoming iq";
			break;
		}
	}
}
}
}
