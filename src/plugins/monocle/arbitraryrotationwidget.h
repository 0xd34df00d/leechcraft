/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_arbitraryrotationwidget.h"

namespace LC
{
namespace Monocle
{
	class ArbitraryRotationWidget : public QWidget
	{
		Q_OBJECT

		Ui::ArbitraryRotationWidget Ui_;
	public:
		ArbitraryRotationWidget (QWidget* = 0);

		double GetValue () const;
	public slots:
		void setValue (double);
	private slots:
		void on_Slider__valueChanged (int);
		void on_Spinbox__valueChanged (double);
		void on_Reset__released ();
	signals:
		void valueChanged (double);
	};
}
}
