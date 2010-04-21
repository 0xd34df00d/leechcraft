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

#include "itemhandlerfont.h"
#include <QLabel>
#include <QFormLayout>
#include <QApplication>
#include <QtDebug>
#include "../fontpicker.h"

namespace LeechCraft
{
	ItemHandlerFont::ItemHandlerFont ()
	{
	}

	ItemHandlerFont::~ItemHandlerFont ()
	{
	}

	bool ItemHandlerFont::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "font";
	}

	void ItemHandlerFont::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		QString labelString = XSD_->GetLabel (item);
		QLabel *label = new QLabel (labelString);
		label->setWordWrap (false);

		FontPicker *picker = new FontPicker (labelString, XSD_);
		picker->setObjectName (item.attribute ("property"));
		picker->SetCurrentFont (XSD_->GetValue (item).value<QFont> ());

		connect (picker,
				SIGNAL (currentFontChanged (const QFont&)),
				this,
				SLOT (updatePreferences ()));

		picker->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		lay->addRow (label, picker);
	}

	QVariant ItemHandlerFont::GetValue (const QDomElement& element,
			QVariant value) const
	{
		if (value.isNull () ||
				!value.canConvert<QFont> ())
		{
			if (element.hasAttribute ("default"))
			{
				QFont font;
				if (!font.fromString (element.attribute ("default")))
					value = QApplication::font ();
			}
			else
				value = QApplication::font ();
		}
		return value;
	}

	void ItemHandlerFont::SetValue (QWidget *widget,
			const QVariant& value) const
	{
		FontPicker *fontPicker = qobject_cast<FontPicker*> (widget);
		if (!fontPicker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a FontPicker"
				<< widget;
			return;
		}
		fontPicker->SetCurrentFont (value.value<QFont> ());
	}

	void ItemHandlerFont::UpdateValue (QDomElement& element,
			const QVariant& value) const
	{
		element.setAttribute ("default", value.value<QFont> ().toString ());
	}

	QVariant ItemHandlerFont::GetValue (QObject *object) const
	{
		FontPicker *fontPicker = qobject_cast<FontPicker*> (object);
		if (!fontPicker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a FontPicker"
				<< object;
			return QVariant ();
		}
		return fontPicker->GetCurrentFont ();
	}
};
