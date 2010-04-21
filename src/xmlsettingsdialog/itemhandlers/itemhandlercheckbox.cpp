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

#include "itemhandlercheckbox.h"
#include <QCheckBox>
#include <QLabel>
#include <QFormLayout>
#include <QtDebug>

namespace LeechCraft
{
	ItemHandlerCheckbox::ItemHandlerCheckbox ()
	{
	}

	ItemHandlerCheckbox::~ItemHandlerCheckbox ()
	{
	}

	bool ItemHandlerCheckbox::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "checkbox";
	}

	void ItemHandlerCheckbox::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		QCheckBox *box = new QCheckBox (XSD_->GetLabel (item));
		box->setObjectName (item.attribute ("property"));

		QVariant value = XSD_->GetValue (item);

		box->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
		connect (box,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (updatePreferences ()));

		box->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		lay->addRow (box);
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

	QVariant ItemHandlerCheckbox::GetValue (QObject *object) const
	{
		QCheckBox *checkbox = qobject_cast<QCheckBox*> (object);
		if (!checkbox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QCheckBox"
				<< object;
			return QVariant ();
		}
		return checkbox->checkState ();
	}
};
