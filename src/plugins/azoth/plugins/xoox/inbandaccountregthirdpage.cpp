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

#include "inbandaccountregthirdpage.h"
#include <QVBoxLayout>
#include <QLabel>
#include "inbandaccountregsecondpage.h"
#include "glooxaccountconfigurationwidget.h"

namespace LeechCraft
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
	{
		setLayout (new QVBoxLayout);
		layout ()->addWidget (StateLabel_);

		connect (SecondPage_,
				SIGNAL (successfulReg ()),
				this,
				SLOT (handleSuccessfulReg ()));
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
		ConfWidget_->SetNick (jid.split ('@', QString::SkipEmptyParts).value (0));
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
