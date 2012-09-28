/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "itemhandlercolor.h"
#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QtDebug>
#include "../colorpicker.h"

namespace LeechCraft
{
	ItemHandlerColor::ItemHandlerColor ()
	{
	}

	ItemHandlerColor::~ItemHandlerColor ()
	{
	}

	bool ItemHandlerColor::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "color";
	}

	void ItemHandlerColor::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		const QString& labelString = XSD_->GetLabel (item);
		QLabel *label = new QLabel (labelString);
		label->setWordWrap (false);

		ColorPicker *picker = new ColorPicker (labelString);
		picker->setObjectName (item.attribute ("property"));
		picker->SetCurrentColor (XSD_->GetValue (item).value<QColor> ());

		connect (picker,
				SIGNAL (currentColorChanged (const QColor&)),
				this,
				SLOT (updatePreferences ()));

		picker->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		int row = lay->rowCount ();
		lay->setColumnMinimumWidth (0, 10);
		lay->setColumnStretch (0, 1);
		lay->setColumnStretch (1, 10);
		lay->addWidget (label, row, 0);
		lay->addWidget (picker, row, 1, Qt::AlignLeft);
	}

	QVariant ItemHandlerColor::GetValue (const QDomElement& item, QVariant value) const
	{
		if (!value.canConvert<QColor> ()
				|| !value.value<QColor> ().isValid ())
			value = QColor (item.attribute ("default"));

		return value;
	}

	void ItemHandlerColor::SetValue (QWidget *widget, const QVariant& value) const
	{
		ColorPicker *colorPicker = qobject_cast<ColorPicker*> (widget);
		if (!colorPicker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a ColorPicker"
				<< widget;
			return;
		}
		colorPicker->SetCurrentColor (value.value<QColor> ());
	}

	void ItemHandlerColor::UpdateValue (QDomElement& element, const QVariant& value) const
	{
		element.setAttribute ("default", value.value<QColor> ().name ());
	}

	QVariant ItemHandlerColor::GetObjectValue (QObject *object) const
	{
		ColorPicker *colorPicker = qobject_cast<ColorPicker*> (object);
		if (!colorPicker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a ColorPicker"
				<< object;
			return QVariant ();
		}
		return colorPicker->GetCurrentColor ();
	}
};
