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

#include "itemhandlercombobox.h"
#include <QLabel>
#include <QFormLayout>
#include <QComboBox>
#include <QtDebug>
#include "../scripter.h"

namespace LeechCraft
{
	ItemHandlerCombobox::ItemHandlerCombobox ()
	{
	}

	ItemHandlerCombobox::~ItemHandlerCombobox ()
	{
	}

	bool ItemHandlerCombobox::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "combobox";
	}

	void ItemHandlerCombobox::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		QComboBox *box = new QComboBox (XSD_);
		box->setObjectName (item.attribute ("property"));
		if (item.hasAttribute ("maxVisibleItems"))
			box->setMaxVisibleItems (item.attribute ("maxVisibleItems").toInt ());

		QDomElement option = item.firstChildElement ("option");
		while (!option.isNull ())
		{
			QList<QImage> images = XSD_->GetImages (option);
			if (images.size ())
			{
				QIcon icon = QIcon (QPixmap::fromImage (images.at (0)));
				box->addItem (icon,
						XSD_->GetLabel (option),
						option.attribute ("name"));
			}
			else
				box->addItem (XSD_->GetLabel (option),
						option.attribute ("name"));

			option = option.nextSiblingElement ("option");
		}

		connect (box,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (updatePreferences ()));

		QDomElement scriptContainer = item.firstChildElement ("scripts");
		if (!scriptContainer.isNull ())
		{
			Scripter scripter (scriptContainer);

			QStringList fromScript = scripter.GetOptions ();
			for (QStringList::const_iterator i = fromScript.begin (),
					end = fromScript.end (); i != end; ++i)
				box->addItem (scripter.HumanReadableOption (*i),
						*i);
		}

		int pos = box->findData (XSD_->GetValue (item));
		if (pos != -1)
			box->setCurrentIndex (pos);
		else
			qWarning () << Q_FUNC_INFO
				<< box
				<< XSD_->GetValue (item)
				<< "not found";

		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		box->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		lay->addRow (label, box);
	}

	void ItemHandlerCombobox::SetValue (QWidget *widget, const QVariant& value) const
	{
		QComboBox *combobox = qobject_cast<QComboBox*> (widget);
		if (!combobox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QComboBox"
				<< widget;
			return;
		}

		int pos = combobox->findData (value);
		if (pos != -1)
			combobox->setCurrentIndex (pos);
		else
			qWarning () << Q_FUNC_INFO
				<< combobox
				<< value
				<< "not found";
	}

	QVariant ItemHandlerCombobox::GetValue (QObject *object) const
	{
		QComboBox *combobox = qobject_cast<QComboBox*> (object);
		if (!combobox)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a QComboBox"
				<< object;
			return QVariant ();
		}
		return combobox->itemData (combobox->currentIndex ());
	}
}
