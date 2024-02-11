/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QtDebug>
#include "radiogroup.h"

namespace LC
{
	RadioGroup::RadioGroup (const QString& title, QWidget *parent)
	: QGroupBox { title, parent }
	, Group_ { *new QButtonGroup { this } }
	, Layout_ { *new QVBoxLayout }
	{
		setLayout (&Layout_);
	}

	void RadioGroup::AddButton (const QString& value, const QString& label, const QString& tooltip, bool isSelected)
	{
		if (Value_.isEmpty ())
			isSelected = true;

		if (isSelected)
			Value_ = value;

		auto button = new QRadioButton { label };
		button->setToolTip (tooltip);
		button->setChecked (isSelected);
		Group_.addButton (button);

		Value2Button_ [value] = button;

		Layout_.addWidget (button);
		connect (button,
				&QRadioButton::toggled,
				this,
				[this, value] (bool enabled)
				{
					if (enabled)
					{
						Value_ = value;
						emit valueChanged ();
					}
				});
	}

	QString RadioGroup::GetValue () const
	{
		return Value_;
	}

	void RadioGroup::SetValue (const QString& value)
	{
		if (const auto button = Value2Button_.value (value))
			button->setChecked (true);
		else
			qWarning () << "could not find button for" << value << "among" << Value2Button_.keys ();
	}
}
