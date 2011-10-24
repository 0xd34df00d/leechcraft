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

#include "itemhandlerpath.h"
#include <QDir>
#include <QLabel>
#include <QGridLayout>
#include <QtDebug>
#include <QDesktopServices>
#include "../filepicker.h"

namespace LeechCraft
{
	ItemHandlerPath::ItemHandlerPath ()
	{
	}

	ItemHandlerPath::~ItemHandlerPath ()
	{
	}

	bool ItemHandlerPath::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "path";
	}

	void ItemHandlerPath::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		FilePicker::Type type = FilePicker::TExistingDirectory;
		if (item.attribute ("pickerType") == "openFileName")
			type = FilePicker::TOpenFileName;
		else if (item.attribute ("pickerType") == "saveFileName")
			type = FilePicker::TSaveFileName;

		FilePicker *picker = new FilePicker (type, XSD_);
		const QVariant& value = XSD_->GetValue (item);
		picker->SetText (value.toString ());
		picker->setObjectName (item.attribute ("property"));
		if (item.attribute ("onCancel") == "clear")
			picker->SetClearOnCancel (true);
		if (item.hasAttribute ("filter"))
			picker->SetFilter (item.attribute ("filter"));

		connect (picker,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (updatePreferences ()));

		picker->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0);
		lay->addWidget (picker, row, 1);
	}

	QVariant ItemHandlerPath::GetValue (const QDomElement& item,
			QVariant value) const
	{
		if (value.isNull () ||
				value.toString ().isEmpty ())
		{
			if (item.hasAttribute ("defaultHomePath") &&
					item.attribute ("defaultHomePath") == "true")
				value = QDir::homePath ();
			else if (item.hasAttribute ("default"))
			{
				QString text = item.attribute ("default");
				QMap<QString, QDesktopServices::StandardLocation> str2loc;
				str2loc ["DOCUMENTS"] = QDesktopServices::DocumentsLocation;
				str2loc ["DESKTOP"] = QDesktopServices::DocumentsLocation;
				str2loc ["MUSIC"] = QDesktopServices::DocumentsLocation;
				str2loc ["MOVIES"] = QDesktopServices::DocumentsLocation;
				Q_FOREACH (const QString& key, str2loc.keys ())
					if (text.startsWith ("{" + key + "}"))
					{
						text.replace (0, key.length () + 2, QDesktopServices::storageLocation (str2loc [key]));
						break;
					}

				value = text;
			}
		}
		return value;
	}

	void ItemHandlerPath::SetValue (QWidget *widget,
			const QVariant& value) const
	{
		FilePicker *picker = qobject_cast<FilePicker*> (widget);
		if (!picker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a FilePicker"
				<< widget;
			return;
		}
		picker->SetText (value.toString ());
	}

	QVariant ItemHandlerPath::GetValue (QObject *object) const
	{
		FilePicker *picker = qobject_cast<FilePicker*> (object);
		if (!picker)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a FilePicker"
				<< object;
			return QVariant ();
		}
		return picker->GetText ();
	}
};
