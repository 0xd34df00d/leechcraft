/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "volumeslider.h"
#include <interfaces/core/iiconthememanager.h>
#include "engine/output.h"
#include "core.h"

namespace LC
{
namespace LMP
{
	VolumeSlider::VolumeSlider (Output *out, QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.Slider_->setOrientation (Qt::Horizontal);
		Ui_.Slider_->setRange (0, 100);
		Ui_.Slider_->setValue (out->GetVolume () * 100);

		connect (Ui_.Slider_,
				SIGNAL (valueChanged (int)),
				out,
				SLOT (setVolume (int)));
		connect (out,
				SIGNAL (volumeChanged (int)),
				Ui_.Slider_,
				SLOT (setValue (int)));

		connect (out,
				SIGNAL (mutedChanged (bool)),
				this,
				SLOT (handleMuted (bool)));
		handleMuted (out->IsMuted ());

		connect (Ui_.MuteButton_,
				SIGNAL (released ()),
				out,
				SLOT (toggleMuted ()));
	}

	void VolumeSlider::handleMuted (bool muted)
	{
		const auto iconName = muted ? "player-volume-muted" : "player-volume";
		Ui_.MuteButton_->setIcon (Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon (iconName));
	}
}
}
