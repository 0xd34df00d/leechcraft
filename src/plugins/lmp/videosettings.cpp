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

