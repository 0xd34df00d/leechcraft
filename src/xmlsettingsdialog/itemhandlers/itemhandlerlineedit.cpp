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

#include "itemhandlerlineedit.h"
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QApplication>
#include <QtDebug>

namespace LeechCraft
{
	ItemHandlerLineEdit::ItemHandlerLineEdit ()
	{
	}

	ItemHandlerLineEdit::~ItemHandlerLineEdit ()
	{
	}

	bool ItemHandlerLineEdit::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "lineedit";
	}

	void ItemHandlerLineEdit::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		QVariant value = XSD_->GetValue (item);

		QLineEdit *edit = new QLineEdit (value.toString ());
		edit->setObjectName (item.attribute ("property"));
		edit->setMinimumWidth (QApplication::fontMetrics ()
				.width ("thisismaybeadefaultsetting"));
		if (item.hasAttribute ("password"))
			edit->setEchoMode (QLineEdit::Password);
		if (item.hasAttribute ("inputMask"))
			edit->setInputMask (item.attribute ("inputMask"));
		connect (edit,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (updatePreferences ()));

		edit->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		lay->addRow (label, edit);
	}

	void ItemHandlerLineEdit::SetValue (QWidget *widget, const QVariant& value) const
	{
		QLineEdit *edit = qobject_cast<QLineEdit*> (widget);
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QLineEdit"
				<< widget;
			return;
		}
		edit->setText (value.toString ());
	}

	QVariant ItemHandlerLineEdit::GetValue (QObject *object) const
	{
		QLineEdit *edit = qobject_cast<QLineEdit*> (object);
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QLineEdit"
				<< object;
			return QVariant ();
		}
		return edit->text ();
	}
};
