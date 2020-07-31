/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlercheckbox.h"
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QtDebug>

namespace LC
{
	bool ItemHandlerCheckbox::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "checkbox";
	}

	void ItemHandlerCheckbox::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QCheckBox *box = new QCheckBox (XSD_->GetLabel (item));
		XSD_->SetTooltip (box, item);
		box->setObjectName (item.attribute ("property"));

		const QVariant& value = XSD_->GetValue (item);

		box->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
		connect (box,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (updatePreferences ()));

		box->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		box->setProperty ("SearchTerms", QStringList (box->text ()));

		lay->addWidget (box, lay->rowCount (), 0, 1, 2, Qt::AlignTop);
	}

	void ItemHandlerCheckbox::SetValue (QWidget *widget,
			const QVariant& value) const
	{
		QCheckBox *checkbox = qobject_cast<QCheckBox*> (widget);
		if (!checkbox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QCheckBox"
				<< widget;
			return;
		}
		checkbox->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
	}

	QVariant ItemHandlerCheckbox::GetObjectValue (QObject *object) const
	{
		QCheckBox *checkbox = qobject_cast<QCheckBox*> (object);
		if (!checkbox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QCheckBox"
				<< object;
			return QVariant ();
		}
		return checkbox->checkState () == Qt::Checked;
	}
}
