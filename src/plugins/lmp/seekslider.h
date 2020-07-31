/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_seekslider.h"

namespace LC
{
namespace LMP
{
	class SourceObject;

	class SeekSlider : public QWidget
	{
		Q_OBJECT

		Ui::SeekSlider Ui_;

		SourceObject * const Source_;

		bool IgnoreNextValChange_ = false;

		bool IsPressed_ = false;
	public:
		explicit SeekSlider (SourceObject*, QWidget* = nullptr);
	private:
		void HandleCurrentPlayTime (qint64);
		void UpdateRanges ();
		void HandleStateChanged ();
	private slots:
		void on_Slider__valueChanged (int);
	};
}
}
