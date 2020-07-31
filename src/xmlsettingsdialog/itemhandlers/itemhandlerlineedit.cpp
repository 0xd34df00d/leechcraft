/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerlineedit.h"
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QApplication>
#include <QtDebug>

namespace LC
{
	bool ItemHandlerLineEdit::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "lineedit";
	}

	void ItemHandlerLineEdit::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		const QVariant& value = XSD_->GetValue (item);

		QLineEdit *edit = new QLineEdit (value.toString ());
		XSD_->SetTooltip (edit, item);
		edit->setObjectName (item.attribute ("property"));
		edit->setMinimumWidth (QApplication::fontMetrics ().horizontalAdvance ("thisismaybeadefaultsetting"));
		if (item.hasAttribute ("password"))
			edit->setEchoMode (QLineEdit::Password);
		if (item.hasAttribute ("inputMask"))
			edit->setInputMask (item.attribute ("inputMask"));
		connect (edit,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (updatePreferences ()));

		edit->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		edit->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignRight);
		lay->addWidget (edit, row, 1);
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

	QVariant ItemHandlerLineEdit::GetObjectValue (QObject *object) const
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
