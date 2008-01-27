#include "commonjobadder.h"

CommonJobAdder::CommonJobAdder (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
}

QString CommonJobAdder::GetString () const
{
	return What_->text ();
}

