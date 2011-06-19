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
#include "inbandaccountregfirstpage.h"

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
		return QWizardPage::isComplete ();
	}
	
	void InBandAccountRegSecondPage::initializePage ()
	{
		QWizardPage::initializePage ();
		
		const QString& server = FirstPage_->GetServerName ();
		ShowMessage (tr ("Please wait while registration "
				"information is fetched from %1...").arg (server));
		
		if (Client_->isConnected ())
			Client_->disconnectFromServer ();
		
		QXmppConfiguration conf;
		conf.setDomain (server);
		conf.setSASLAuthMechanism (QXmppConfiguration::SASLAnonymous);
		conf.setIgnoreAuth (true);
		Client_->connectToServer (conf);
	}
	
	void InBandAccountRegSecondPage::ShowMessage (const QString& msg)
	{
		qDeleteAll (findChildren<QWidget*> ());
		
		layout ()->addWidget (new QLabel (msg));
	}
	
	void InBandAccountRegSecondPage::handleConnected ()
	{
		ShowMessage ("Connected!");

		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsRegister);

		QXmppIq iq;
		iq.setExtensions (queryElem);
		Client_->sendPacket (iq);
	}
	
	void InBandAccountRegSecondPage::handleError (QXmppClient::Error error)
	{
		ShowMessage (tr ("Error"));
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
			ShowMessage (tr ("Service unavailable"));
			return;
		}
		
		qDeleteAll (findChildren<QWidget*> ());
		QWidget *widget = LFB_.CreateForm (queryElem);
		layout ()->addWidget (widget);
	}
}
}
}
