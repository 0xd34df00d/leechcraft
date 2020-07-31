/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eqbandwidget.h"
#include <cmath>
#include "bandinfo.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	namespace
	{
		const double SliderPrecision = 10;
	}

	EqBandWidget::EqBandWidget (const BandInfo& info, QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		Ui_.FreqBox_->setValue (info.Freq_);
		Ui_.FreqBox_->setSuffix (" " + tr ("Hz"));

		Ui_.GainSlider_->setRange (Ui_.GainBox_->minimum () * SliderPrecision,
				Ui_.GainBox_->maximum () * SliderPrecision);

		connect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SLOT (setGainSliderValue (double)));
		connect (Ui_.GainSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (setGainBoxValue (int)));

		connect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SIGNAL (valueChanged (double)));
	}

	void EqBandWidget::SetGain (double value)
	{
		disconnect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SIGNAL (valueChanged (double)));
		Ui_.GainBox_->setValue (value);
		connect (Ui_.GainBox_,
				SIGNAL (valueChanged (double)),
				this,
				SIGNAL (valueChanged (double)));
	}

	double EqBandWidget::GetGain ()
	{
		return Ui_.GainBox_->value ();
	}

	double EqBandWidget::GetFrequency () const
	{
		return Ui_.FreqBox_->value ();
	}

	void EqBandWidget::setGainSliderValue (double value)
	{
		disconnect (Ui_.GainSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (setGainBoxValue (int)));

		Ui_.GainSlider_->setValue (std::round (value * SliderPrecision));

		connect (Ui_.GainSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (setGainBoxValue (int)));
	}

	void EqBandWidget::setGainBoxValue (int value)
	{
		Ui_.GainBox_->setValue (value / SliderPrecision);
	}
}
}
}
