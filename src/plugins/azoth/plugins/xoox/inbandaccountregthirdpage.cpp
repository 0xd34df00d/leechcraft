/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inbandaccountregthirdpage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QtDebug>
#include "inbandaccountregsecondpage.h"
#include "glooxaccountconfigurationwidget.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	InBandAccountRegThirdPage::InBandAccountRegThirdPage (InBandAccountRegSecondPage *second, QWidget *parent)
	: QWizardPage (parent)
	, SecondPage_ (second)
	, ConfWidget_ (0)
	, StateLabel_ (new QLabel ())
	, RegState_ (RSIdle)
	{
		setLayout (new QVBoxLayout);
		layout ()->addWidget (StateLabel_);

		connect (SecondPage_,
				SIGNAL (successfulReg ()),
				this,
				SLOT (handleSuccessfulReg ()));
		connect (SecondPage_,
				SIGNAL (regError (QString)),
				this,
				SLOT (handleRegError (QString)));
	}

	void InBandAccountRegThirdPage::SetConfWidget (GlooxAccountConfigurationWidget *w)
	{
		ConfWidget_ = w;
	}

	bool InBandAccountRegThirdPage::isComplete () const
	{
		switch (RegState_)
		{
		case RSError:
		case RSIdle:
		case RSAwaitingResult:
			return false;
		case RSSuccess:
			return true;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown state"
					<< RegState_;
			return true;
		}
	}

	void InBandAccountRegThirdPage::initializePage ()
	{
		SecondPage_->Register ();

		StateLabel_->setText (tr ("Awaiting registration result..."));
		SetState (RSAwaitingResult);
	}

	void InBandAccountRegThirdPage::SetState (InBandAccountRegThirdPage::RegState state)
	{
		RegState_ = state;
		emit completeChanged ();
	}

	void InBandAccountRegThirdPage::handleSuccessfulReg ()
	{
		StateLabel_->setText (tr ("Registration completed successfully. "
				"You may now further configure account properties."));
		const QString& jid = SecondPage_->GetJID ();
		ConfWidget_->SetJID (jid);
		ConfWidget_->SetNick (jid.split ('@', Qt::SkipEmptyParts).value (0));
		SetState (RSSuccess);
	}

	void InBandAccountRegThirdPage::handleRegError (const QString& msg)
	{
		StateLabel_->setText (tr ("Registration failed: %1.").arg (msg));
		SetState (RSError);
	}
}
}
}
