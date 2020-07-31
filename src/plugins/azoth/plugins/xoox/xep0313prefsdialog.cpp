/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xep0313prefsdialog.h"
#include "xeps/xep0313manager.h"
#include "xeps/xep0313prefiq.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	Xep0313PrefsDialog::Xep0313PrefsDialog (Xep0313Manager *mgr, QWidget *parent)
	: QDialog { parent }
	, Manager_ { mgr }
	{
		Ui_.setupUi (this);

		connect (mgr,
				SIGNAL (gotPrefs (Xep0313PrefIq)),
				this,
				SLOT (handlePrefs (Xep0313PrefIq)));
		mgr->RequestPrefs ();

		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (updatePrefs ()));

		setAttribute (Qt::WA_DeleteOnClose);
	}

	void Xep0313PrefsDialog::updatePrefs ()
	{
		Xep0313PrefIq iq;
		const auto idx = Ui_.DefaultMode_->currentIndex ();
		iq.SetDefaultPolicy (static_cast<Xep0313PrefIq::DefaultPolicy> (idx));

		iq.SetAllowed (Ui_.Always_->toPlainText ().split ('\n', Qt::SkipEmptyParts));
		iq.SetForbidden (Ui_.Never_->toPlainText ().split ('\n', Qt::SkipEmptyParts));

		Manager_->SetPrefs (iq);
	}

	void Xep0313PrefsDialog::handlePrefs (const Xep0313PrefIq& iq)
	{
		Ui_.DefaultMode_->setCurrentIndex (static_cast<int> (iq.GetDefaultPolicy ()));

		Ui_.Always_->setPlainText (iq.GetAllowed ().join ("\n"));
		Ui_.Never_->setPlainText (iq.GetForbidden ().join ("\n"));
	}
}
}
}
