/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "itemhandlerradio.h"
#include <QLabel>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QtDebug>
#include "../radiogroup.h"

namespace LeechCraft
{
	ItemHandlerRadio::ItemHandlerRadio ()
	{
	}

	ItemHandlerRadio::~ItemHandlerRadio ()
	{
	}

	bool ItemHandlerRadio::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "radio";
	}

	void ItemHandlerRadio::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		RadioGroup *group = new RadioGroup (XSD_);
		group->setObjectName (item.attribute ("property"));

		QDomElement option = item.firstChildElement ("option");
		while (!option.isNull ())
		{
			QRadioButton *button = new QRadioButton (XSD_->GetLabel (option));
			button->setObjectName (option.attribute ("name"));
			group->AddButton (button,
					option.hasAttribute ("default") &&
					option.attribute ("default") == "true");
			option = option.nextSiblingElement ("option");
		}

		QVariant value = XSD_->GetValue (item);

		connect (group,
				SIGNAL (valueChanged ()),
				this,
				SLOT (updatePreferences ()));

		QGroupBox *box = new QGroupBox (XSD_->GetLabel (item));
		QVBoxLayout *layout = new QVBoxLayout ();
		box->setLayout (layout);
		layout->addWidget (group);

		group->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		lay->addRow (box);
	}

	void ItemHandlerRadio::SetValue (QWidget *widget, const QVariant& value) const
	{
		RadioGroup *radiogroup = qobject_cast<RadioGroup*> (widget);
		if (!radiogroup)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a RadioGroup"
				<< widget;
			return;
		}
		radiogroup->SetValue (value.toString ());
	}

	QVariant ItemHandlerRadio::GetValue (QObject *object) const
	{
		RadioGroup *radiogroup = qobject_cast<RadioGroup*> (object);
		if (!radiogroup)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a RadioGroup"
				<< object;
			return QVariant ();
		}
		return radiogroup->GetValue ();
	}
};
