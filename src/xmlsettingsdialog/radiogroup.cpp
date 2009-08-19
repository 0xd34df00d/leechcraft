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

#include <QButtonGroup>
#include <QtDebug>
#include <QRadioButton>
#include <QVBoxLayout>
#include "radiogroup.h"

using namespace LeechCraft;

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

void RadioGroup::SetValue (const QString& value)
{
	QRadioButton *button = findChild<QRadioButton*> (value);
	if (!button)
	{
		qWarning () << Q_FUNC_INFO << "could not find button for" << value;
		return;
	}
	button->setChecked (true);
}

void RadioGroup::handleToggled (bool value)
{
	if (value)
	{
		Value_ = sender ()->objectName ();
		emit valueChanged ();
	}
}

