/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include "xmppbobmanager.h"
#include "inbandaccountregfirstpage.h"
#include "util.h"
#include "regformhandlerwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	namespace
	{
		QXmppClient* MakeClient (InBandAccountRegSecondPage *page)
		{
			auto client = new QXmppClient (page);
			Q_FOREACH (auto ext, client->extensions ())
				client->removeExtension (ext);

			client->addExtension (new XMPPBobManager);

			return client;
		}
	}

	InBandAccountRegSecondPage::InBandAccountRegSecondPage (InBandAccountRegFirstPage *first, QWidget *parent)
	: QWizardPage (parent)
	, Client_ (MakeClient (this))
	, RegForm_ (new RegFormHandlerWidget (Client_))
	, FirstPage_ (first)
	{
		setLayout (new QVBoxLayout);
		layout ()->addWidget (RegForm_);

		connect (Client_,
				SIGNAL (connected ()),
				this,
				SLOT (handleConnected ()));

		connect (RegForm_,
				SIGNAL (completeChanged ()),
				this,
				SIGNAL (completeChanged ()));
		connect (RegForm_,
				SIGNAL (successfulReg ()),
				this,
				SIGNAL (successfulReg ()));
		connect (RegForm_,
				SIGNAL (regError (QString)),
				this,
				SIGNAL (regError (QString)));
	}

	void InBandAccountRegSecondPage::Register ()
	{
		RegForm_->Register ();
	}

	QString InBandAccountRegSecondPage::GetJID () const
	{
		return RegForm_->GetUser () + '@' + FirstPage_->GetServerName ();
	}

	QString InBandAccountRegSecondPage::GetPassword () const
	{
		return RegForm_->GetPassword ();
	}

	bool InBandAccountRegSecondPage::isComplete () const
	{
		return RegForm_->IsComplete ();
	}

	void InBandAccountRegSecondPage::initializePage ()
	{
		QWizardPage::initializePage ();

		const QString& server = FirstPage_->GetServerName ();

		if (Client_->isConnected ())
			Client_->disconnectFromServer ();

		QXmppConfiguration conf;
		conf.setDomain (server);
		conf.setUseNonSASLAuthentication (false);
		conf.setUseSASLAuthentication (false);
		Client_->connectToServer (conf);
	}

	void InBandAccountRegSecondPage::handleConnected ()
	{
		RegForm_->SendRequest ();
	}
}
}
}
