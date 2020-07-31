/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "delegatebuttongroup.h"
#include <QAbstractButton>
#include <QtDebug>

namespace LC
{
namespace LackMan
{
	DelegateButtonGroup::DelegateButtonGroup (QObject *parent)
	: QObject (parent)
	{
	}

	void DelegateButtonGroup::AddButton (QAbstractButton *button)
	{
		Buttons_ << button;
		connect (button,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleButtonToggled (bool)));
	}

	void DelegateButtonGroup::handleButtonToggled (bool toggled)
	{
		QAbstractButton *button = qobject_cast<QAbstractButton*> (sender ());
		if (!button)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a QAbstractButton*"
					<< sender ();
			return;
		}

		if (toggled)
			for (auto otherButton : Buttons_)
				if (otherButton != button && otherButton->isChecked ())
					otherButton->setChecked (false);
	}
}
}
