/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_volumeslider.h"

namespace LC
{
namespace LMP
{
	class Output;

	class VolumeSlider : public QWidget
	{
		Q_OBJECT

		Ui::VolumeSlider Ui_;
	public:
		VolumeSlider (Output*, QWidget* = 0);
	private slots:
		void handleMuted (bool);
	};
}
}
