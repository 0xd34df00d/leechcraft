/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "videosettings.h"

using namespace LeechCraft::Plugins::LMP;

VideoSettings::VideoSettings (qreal b, qreal c, qreal h, qreal s, QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.Brightness_->setValue (b * 100);
	Ui_.Contrast_->setValue (c * 100);
	Ui_.Hue_->setValue (h * 100);
	Ui_.Saturation_->setValue (s * 100);
}

VideoSettings::~VideoSettings ()
{
}

qreal VideoSettings::Brightness () const
{
	qreal result = Ui_.Brightness_->value ();
	result /= 100;
	return result;
}

qreal VideoSettings::Contrast () const
{
	qreal result = Ui_.Contrast_->value ();
	result /= 100;
	return result;
}

qreal VideoSettings::Hue () const
{
	qreal result = Ui_.Hue_->value ();
	result /= 100;
	return result;
}

qreal VideoSettings::Saturation () const
{
	qreal result = Ui_.Saturation_->value ();
	result /= 100;
	return result;
}

