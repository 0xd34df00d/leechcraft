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

		const QVariant& value = XSD_->GetValue (item);

		QTextEdit *edit = new QTextEdit ();
		XSD_->SetTooltip (edit, item);
		edit->setPlainText (value.toStringList ().join ("\n"));
		edit->setObjectName (item.attribute ("property"));
		edit->setMinimumWidth (QApplication::fontMetrics ()
				.width ("thisismaybeadefaultsetting"));
		connect (edit,
				SIGNAL (textChanged ()),
				this,
				SLOT (updatePreferences ()));

		edit->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		edit->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		QString pos = item.attribute ("position");
		if (pos == "bottom")
		{
			lay->addWidget (label, row, 0, Qt::AlignLeft);
			lay->addWidget (edit, row + 1, 0);
		}
		else if (pos == "right")
		{
			lay->addWidget (label, row, 0, Qt::AlignRight | Qt::AlignTop);
			lay->addWidget (edit, row, 1);
		}
		else if (pos == "left")
		{
			lay->addWidget (label, row, 1, Qt::AlignLeft | Qt::AlignTop);
			lay->addWidget (edit, row, 0);
		}
		else if (pos == "top")
		{
			lay->addWidget (edit, row, 0);
			lay->addWidget (label, row + 1, 0, Qt::AlignLeft);
		}
		else
		{
			lay->addWidget (label, row, 0, Qt::AlignRight | Qt::AlignTop);
			lay->addWidget (edit, row, 1);
		}

		lay->setContentsMargins (2, 2, 2, 2);
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
		edit->setPlainText (value.toStringList ().join ("\n"));
	}

	QVariant ItemHandlerMultiLine::GetValue (const QDomElement& item,
			QVariant) const
	{
		QString def = item.attribute ("default");
		if (item.attribute ("translatable") == "true")
			def = QCoreApplication::translate (qPrintable (XSD_->GetBasename ()),
					def.toUtf8 ().constData ());
		return def;
	}

	void ItemHandlerMultiLine::UpdateValue (QDomElement& element,
			const QVariant& value) const
	{
		element.setAttribute ("default", value.toString ());
	}

	QVariant ItemHandlerMultiLine::GetObjectValue (QObject *object) const
	{
		QTextEdit *edit = qobject_cast<QTextEdit*> (object);
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QTextEdit"
				<< object;
			return QVariant ();
		}
		return edit->toPlainText ().split ('\n', QString::SkipEmptyParts);
	}
}
