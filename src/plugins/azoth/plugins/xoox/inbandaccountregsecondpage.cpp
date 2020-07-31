/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inbandaccountregsecondpage.h"
#include <QVBoxLayout>
#include <util/network/socketerrorstrings.h>
#include <util/util.h>
#include <util/sll/slotclosure.h>
#include "xeps/xmppbobmanager.h"
#include "inbandaccountregfirstpage.h"
#include "util.h"
#include "regformhandlerwidget.h"
#include "sslerrorshandler.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	namespace
	{
		QXmppClient* MakeClient (InBandAccountRegSecondPage *page)
		{
			auto client = new QXmppClient { page };
			for (auto ext : client->extensions ())
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
		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleClientError (QXmppClient::Error)));

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

		const auto sslHandler = new SslErrorsHandler { Client_ };
		connect (sslHandler,
				SIGNAL (sslErrors (QList<QSslError>, ICanHaveSslErrors::ISslErrorsReaction_ptr)),
				this,
				SIGNAL (sslErrors (QList<QSslError>, ICanHaveSslErrors::ISslErrorsReaction_ptr)));

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { SslAborted_ = true; },
			sslHandler,
			SIGNAL (aborted ()),
			sslHandler
		};
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

	QObject* InBandAccountRegSecondPage::GetQObject ()
	{
		return this;
	}

	bool InBandAccountRegSecondPage::isComplete () const
	{
		return RegForm_->IsComplete ();
	}

	void InBandAccountRegSecondPage::initializePage ()
	{
		QWizardPage::initializePage ();

		SslAborted_ = false;

		Reinitialize ();
	}

	void InBandAccountRegSecondPage::Reinitialize ()
	{
		const auto& server = FirstPage_->GetServerName ();

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

	void InBandAccountRegSecondPage::handleClientError (QXmppClient::Error error)
	{
		qWarning () << Q_FUNC_INFO
				<< error
				<< Client_->socketError ()
				<< Client_->xmppStreamError ();

		if (error == QXmppClient::SocketError && SslAborted_)
			return;

		if (error == QXmppClient::SocketError &&
				wizard ()->currentPage () == this)
			Reinitialize ();
	}
}
}
}
