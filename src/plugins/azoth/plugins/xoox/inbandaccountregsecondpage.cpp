/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
	, FirstPage_ (first)
	, Widget_ (0)
	, State_ (SIdle)
	{
		Q_FOREACH (QXmppClientExtension *ext, Client_->extensions ())
			Client_->removeExtension (ext);

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
	
	bool InBandAccountRegSecondPage::isComplete () const
	{
		switch (State_)
		{
		case SError:
		case SIdle:
		case SConnecting:
		case SFetchingForm:
			return false;
		case SAwaitingUserInput:
			if (!Widget_)
				return false;
			Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ())
				if (edit->text ().isEmpty ())
					return false;
			return true;
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
		}
		
		ShowMessage (msg);
		
		SetState (SError);
	}
	
	void InBandAccountRegSecondPage::handleIqReceived (const QXmppIq& iq)
	{
		QXmppElement queryElem;
		Q_FOREACH (const QXmppElement& elem, iq.extensions ())
			if (elem.tagName () == "query" &&
					elem.attribute ("xmlns") == NsRegister)
			{
				queryElem = elem;
				break;
			}

		if (queryElem.isNull ())
		{
			SetState (SError);
			ShowMessage (tr ("Service unavailable"));
			return;
		}

		qDeleteAll (findChildren<QWidget*> ());

		const QXmppElement& formElem = queryElem.firstChildElement ("x");
		QWidget *widget = 0;
		if (formElem.attribute ("xmlns") == NsRegister &&
				formElem.attribute ("type") == "form")
		{
			QXmppDataForm form;
			form.parse (Util::XmppElem2DomElem (formElem));
			widget = FB_.CreateForm (form);
		}
		else
			widget = LFB_.CreateForm (queryElem);

		if (!widget)
		{
			SetState (SError);
			qWarning () << Q_FUNC_INFO
					<< "got null widget";
			return;
		}

		layout ()->addWidget (widget);
		Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ())
			connect (edit,
					SIGNAL (textChanged (const QString&)),
					this,
					SIGNAL (completeChanged ()));
		
		SetState (SAwaitingUserInput);
	}
}
}
}
