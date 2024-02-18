/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QHBoxLayout>
#include <QVariant>
#include <QSpinBox>
#include "rangewidget.h"

namespace LC
{
	RangeWidget::RangeWidget (QWidget *parent)
	: QWidget { parent }
	, Lower_ { *new QSpinBox }
	, Higher_ { *new QSpinBox }
	{
		const auto lay = new QHBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		lay->addWidget (&Lower_);
		lay->addWidget (&Higher_);
		lay->addStretch (1);
		setLayout (lay);

		connect (&Lower_,
				&QSpinBox::valueChanged,
				&Higher_,
				&QSpinBox::setMinimum);
		connect (&Higher_,
				&QSpinBox::valueChanged,
				&Lower_,
				&QSpinBox::setMaximum);
		connect (&Lower_,
				&QSpinBox::valueChanged,
				this,
				&RangeWidget::changed);
		connect (&Higher_,
				&QSpinBox::valueChanged,
				this,
				&RangeWidget::changed);
	}

	void RangeWidget::SetBounds (int min, int max)
	{
		Lower_.setRange (min, max);
		Higher_.setRange (min, max);
	}

	void RangeWidget::SetRange (const QVariant& variant)
	{
		const auto& list = variant.toList ();
		if (list.size () != 2)
			return;

		const auto low = list [0].toInt ();
		const auto high = list [1].toInt ();
		if (low > high)
			return;

		Lower_.setValue (low);
		Higher_.setValue (high);
	}

	QVariant RangeWidget::GetRange () const
	{
		return QList<QVariant> { Lower_.value (), Higher_.value () };
	}
}
