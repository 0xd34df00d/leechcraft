/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "itemhandlermultiline.h"
#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include <QApplication>
#include <QtDebug>

namespace LeechCraft
{
	ItemHandlerMultiLine::ItemHandlerMultiLine ()
	{
	}

	ItemHandlerMultiLine::~ItemHandlerMultiLine ()
	{
	}

	bool ItemHandlerMultiLine::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "multiline";
	}

	void ItemHandlerMultiLine::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		QVariant value = XSD_->GetValue (item);

		QTextEdit *edit = new QTextEdit (value.toString ());
		edit->setObjectName (item.attribute ("property"));
		edit->setMinimumWidth (QApplication::fontMetrics ()
				.width ("thisismaybeadefaultsetting"));
		connect (edit,
				SIGNAL (textChanged ()),
				this,
				SLOT (updatePreferences ()));

		edit->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignRight);
		lay->addWidget (edit, row, 1);
	}

	void ItemHandlerMultiLine::SetValue (QWidget *widget, const QVariant& value) const
	{
		QTextEdit *edit = qobject_cast<QTextEdit*> (widget);
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QTextEdit"
				<< widget;
			return;
		}
		edit->setText (value.toString ());
	}

	QVariant ItemHandlerMultiLine::GetValue (QObject *object) const
	{
		QTextEdit *edit = qobject_cast<QTextEdit*> (object);
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QTextEdit"
				<< object;
			return QVariant ();
		}
		return edit->toPlainText ();
	}
}
