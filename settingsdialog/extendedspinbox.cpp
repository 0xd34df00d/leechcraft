#include "extendedspinbox.h"

ExtendedSpinbox::ExtendedSpinbox (QWidget *parent)
: QSpinBox (parent)
, MinimumShift_ (0)
, MaximumShift_ (0)
{
}

void ExtendedSpinbox::SetMinimumShift (int value)
{
	MinimumShift_ = value;
}

void ExtendedSpinbox::SetMaximumShift (int value)
{
	MaximumShift_ = value;
}

void ExtendedSpinbox::changeMinimum (int value)
{
	setMinimum (value + MinimumShift_);
}

void ExtendedSpinbox::changeMaximum (int value)
{
	setMaximum (value + MaximumShift_);
}

