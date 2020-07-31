/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlermultiline.h"
#include <QLabel>
#include <QTextEdit>
#include <QGridLayout>
#include <QApplication>
#include <QtDebug>

namespace LC
{
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
		edit->setMinimumWidth (QApplication::fontMetrics ().horizontalAdvance ("thisismaybeadefaultsetting"));
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
		return edit->toPlainText ().split ('\n', Qt::SkipEmptyParts);
	}
}
