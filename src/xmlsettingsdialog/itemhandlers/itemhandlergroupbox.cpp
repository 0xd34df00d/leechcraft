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

#include "itemhandlergroupbox.h"
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QtDebug>

namespace LeechCraft
{
	ItemHandlerGroupbox::ItemHandlerGroupbox ()
	{
	}

	ItemHandlerGroupbox::~ItemHandlerGroupbox ()
	{
	}

	bool ItemHandlerGroupbox::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "groupbox" &&
				element.attribute ("checkable") == "true";
	}

	void ItemHandlerGroupbox::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGroupBox *box = new QGroupBox (XSD_->GetLabel (item));
		box->setObjectName (item.attribute ("property"));
		QGridLayout *groupLayout = new QGridLayout ();
		groupLayout->setContentsMargins (2, 2, 2, 2);
		box->setLayout (groupLayout);
		box->setCheckable (true);

		const QVariant& value = XSD_->GetValue (item);

		box->setChecked (value.toBool ());
		connect (box,
				SIGNAL (toggled (bool)),
				this,
				SLOT (updatePreferences ()));
		box->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		XSD_->ParseEntity (item, box);

		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		lay->addWidget (box, lay->rowCount (), 0, 1, 2);
	}

	void ItemHandlerGroupbox::SetValue (QWidget *widget,
			const QVariant& value) const
	{
		QGroupBox *groupbox = qobject_cast<QGroupBox*> (widget);
		if (!groupbox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QGroupBox"
				<< widget;
			return;
		}
		groupbox->setChecked (value.toBool ());
	}

	QVariant ItemHandlerGroupbox::GetValue (QObject *object) const
	{
		QGroupBox *groupbox = qobject_cast<QGroupBox*> (object);
		if (!groupbox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QGroupBox"
				<< object;
			return QVariant ();
		}
		return groupbox->isChecked ();
	}
};
