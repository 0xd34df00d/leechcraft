/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_eqbandwidget.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	struct BandInfo;

	class EqBandWidget : public QWidget
	{
		Q_OBJECT

		Ui::EqBandWidget Ui_;
	public:
		EqBandWidget (const BandInfo&, QWidget* = 0);

		void SetGain (double);
		double GetGain ();

		double GetFrequency () const;
	private slots:
		void setGainSliderValue (double);
		void setGainBoxValue (int);
	signals:
		void valueChanged (double);
	};
}
}
}
