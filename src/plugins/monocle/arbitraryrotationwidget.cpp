/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "arbitraryrotationwidget.h"

namespace LC
{
namespace Monocle
{
	ArbitraryRotationWidget::ArbitraryRotationWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	double ArbitraryRotationWidget::GetValue () const
	{
		return Ui_.Spinbox_->value ();
	}

	void ArbitraryRotationWidget::setValue (double val)
	{
		Ui_.Slider_->setValue (val);
		Ui_.Spinbox_->setValue (val);
	}

	void ArbitraryRotationWidget::on_Slider__valueChanged (int val)
	{
		Ui_.Spinbox_->setValue (val);
	}

	void ArbitraryRotationWidget::on_Spinbox__valueChanged (double val)
	{
		Ui_.Slider_->setValue (val);
		emit valueChanged (val);
	}

	void ArbitraryRotationWidget::on_Reset__released ()
	{
		setValue (0);
	}
}
}
