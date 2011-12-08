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

#include "itemhandlercombobox.h"
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QtDebug>
#include "../scripter.h"
#include "../itemhandlerfactory.h"

namespace LeechCraft
{
	ItemHandlerCombobox::ItemHandlerCombobox (ItemHandlerFactory *factory)
	: Factory_ (factory)
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
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());

		QHBoxLayout *hboxLay = new QHBoxLayout;
		QComboBox *box = new QComboBox (XSD_);

		hboxLay->addWidget (box);

		XSD_->SetTooltip (box, item);
		box->setObjectName (item.attribute ("property"));
		if (item.hasAttribute ("maxVisibleItems"))
			box->setMaxVisibleItems (item.attribute ("maxVisibleItems").toInt ());

		bool mayHaveDataSource = item.hasAttribute ("mayHaveDataSource") &&
				item.attribute ("mayHaveDataSource").toLower () == "true";
		if (mayHaveDataSource)
		{
			const QString& prop = item.attribute ("property");
			Factory_->RegisterDatasourceSetter (prop,
					[this] (const QString& str, QAbstractItemModel *m, Util::XmlSettingsDialog *xsd)
						{ SetDataSource (str, m, xsd); });
			Propname2Combobox_ [prop] = box;
			Propname2Item_ [prop] = item;
		}

		hboxLay->addStretch ();

		if (item.hasAttribute ("moreThisStuff"))
		{
			QPushButton *moreButt = new QPushButton (tr ("More stuff..."));
			hboxLay->addWidget (moreButt);

			moreButt->setObjectName (item.attribute ("moreThisStuff"));
			connect (moreButt,
					SIGNAL (released ()),
					XSD_,
					SLOT (handleMoreThisStuffRequested ()));
		}

		QDomElement option = item.firstChildElement ("option");
		while (!option.isNull ())
		{
			const QList<QImage>& images = XSD_->GetImages (option);
			if (images.size ())
			{
				const QIcon& icon = QIcon (QPixmap::fromImage (images.at (0)));
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
			Q_FOREACH (const QString& elm, scripter.GetOptions ())
				box->addItem (scripter.HumanReadableOption (elm),
						elm);
		}

		int pos = box->findData (XSD_->GetValue (item));
		if (pos != -1)
			box->setCurrentIndex (pos);
		else if (!mayHaveDataSource)
			qWarning () << Q_FUNC_INFO
				<< box
				<< XSD_->GetValue (item)
				<< "not found (and this item may not have a datasource)";

		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		box->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignRight);
		lay->addLayout (hboxLay, row, 1);
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
		if (pos == -1)
		{
			QString text = value.toString ();
			if (!text.isNull ())
				pos = combobox->findText (text);
		}

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
		QVariant result = combobox->itemData (combobox->currentIndex ());
		if (result.isNull ())
			result = combobox->currentText ();
		return result;
	}

	void ItemHandlerCombobox::SetDataSource (const QString& prop, QAbstractItemModel *model, Util::XmlSettingsDialog *xsd)
	{
		QComboBox *box = Propname2Combobox_ [prop];
		if (!box)
		{
			qWarning () << Q_FUNC_INFO
					<< "combobox for property"
					<< prop
					<< "not found";
			return;
		}

		box->setModel (model);

		const QVariant& data = xsd->GetValue (Propname2Item_ [prop]);
		int pos = box->findData (data);
		if (pos == -1)
		{
			QString text = data.toString ();
			if (!text.isNull ())
				pos = box->findText (text);
		}

		if (pos != -1)
			box->setCurrentIndex (pos);
		else
			qWarning () << Q_FUNC_INFO
				<< box
				<< data
				<< "not found";
	}
}
