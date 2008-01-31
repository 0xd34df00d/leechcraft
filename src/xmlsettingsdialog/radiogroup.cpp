#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include "radiogroup.h"

RadioGroup::RadioGroup (QWidget *parent)
: QWidget (parent)
{
	Group_ = new QButtonGroup (this);
	setLayout (new QVBoxLayout);
}

void RadioGroup::AddButton (QRadioButton *button, bool def)
{
	if (def)
		Value_ = button->objectName ();
	button->setChecked (def);
	Group_->addButton (button);
	qobject_cast<QVBoxLayout*> (layout ())->addWidget (button);
	connect (button, SIGNAL (toggled (bool)), this, SLOT (handleToggled (bool)));
}

QString RadioGroup::GetValue () const
{
	return Value_;
}

void RadioGroup::handleToggled (bool value)
{
	if (value)
	{
		Value_ = sender ()->objectName ();
		emit valueChanged ();
	}
}

