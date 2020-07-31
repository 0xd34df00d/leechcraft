/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerfont.h"
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <QtDebug>
#include "../fontpicker.h"

namespace LC
{
	bool ItemHandlerFont::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "font";
	}

	void ItemHandlerFont::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		const QString& labelString = XSD_->GetLabel (item);
		QLabel *label = new QLabel (labelString);
		label->setWordWrap (false);

		FontPicker *picker = new FontPicker (labelString, XSD_->GetWidget ());
		picker->setObjectName (item.attribute ("property"));
		picker->SetCurrentFont (XSD_->GetValue (item).value<QFont> ());

		connect (picker,
				SIGNAL (currentFontChanged (const QFont&)),
				this,
				SLOT (updatePreferences ()));

		picker->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		picker->setProperty ("SearchTerms", labelString);

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0);
		lay->addWidget (picker, row, 1);
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
				const QString& defStr = element.attribute ("default");
				if (font.fromString (defStr))
					value = font;
				else
					value = QFont (defStr);
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

	QVariant ItemHandlerFont::GetObjectValue (QObject *object) const
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
