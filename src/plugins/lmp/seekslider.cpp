/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "seekslider.h"
#include <cmath>
#include <util/util.h>
#include "engine/sourceobject.h"

namespace LC
{
namespace LMP
{
	SeekSlider::SeekSlider (SourceObject *source, QWidget *parent)
	: QWidget (parent)
	, Source_ (source)
	{
		Ui_.setupUi (this);

		connect (source,
				&SourceObject::currentSourceChanged,
				this,
				&SeekSlider::UpdateRanges);
		connect (source,
				&SourceObject::totalTimeChanged,
				this,
				&SeekSlider::UpdateRanges);
		connect (source,
				&SourceObject::tick,
				this,
				&SeekSlider::HandleCurrentPlayTime);
		connect (source,
				&SourceObject::stateChanged,
				this,
				&SeekSlider::HandleStateChanged);

		connect (Ui_.Slider_,
				&QSlider::sliderPressed,
				[this] { IsPressed_ = true; });
		connect (Ui_.Slider_,
				&QSlider::sliderReleased,
				[this] { IsPressed_ = false; });
	}

	void SeekSlider::HandleCurrentPlayTime (qint64 time)
	{
		auto niceTime = [] (qint64 time)
		{
			if (!time)
				return QString {};

			auto played = Util::MakeTimeFromLong (time / 1000);
			if (played.startsWith ("00:"))
				played = played.mid (3);
			return played;
		};
		Ui_.Played_->setText (niceTime (time));

		const auto remaining = Source_->GetRemainingTime ();
		Ui_.Remaining_->setText (remaining < 0 ? QString () : niceTime (remaining));

		if (!IsPressed_)
			Ui_.Slider_->setValue (time / 1000);
	}

	void SeekSlider::UpdateRanges ()
	{
		const auto newMax = Source_->GetTotalTime () / 1000;
		if (newMax <= Ui_.Slider_->value ())
			IgnoreNextValChange_ = true;

		Ui_.Slider_->setMaximum (newMax);
	}

	void SeekSlider::HandleStateChanged ()
	{
		const auto state = Source_->GetState ();
		switch (state)
		{
		case SourceState::Buffering:
		case SourceState::Playing:
		case SourceState::Paused:
			UpdateRanges ();
			HandleCurrentPlayTime (Source_->GetCurrentTime ());
			Ui_.Slider_->setEnabled (true);
			break;
		default:
			Ui_.Slider_->setRange (0, 0);
			Ui_.Slider_->setEnabled (false);
			Ui_.Played_->setText ({});
			Ui_.Remaining_->setText ({});
			break;
		}
	}

	void SeekSlider::on_Slider__valueChanged (int value)
	{
		value *= 1000;
		const double diff = value - Source_->GetCurrentTime ();
		if (std::abs (diff) < 1500 || IgnoreNextValChange_)
		{
			IgnoreNextValChange_ = false;
			return;
		}

		Source_->Seek (value);
	}
}
}
