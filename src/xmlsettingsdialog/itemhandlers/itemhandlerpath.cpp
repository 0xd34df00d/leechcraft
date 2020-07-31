/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerpath.h"
#include <QDir>
#include <QLabel>
#include <QGridLayout>
#include <QtDebug>
#include <QStandardPaths>
#include <util/sys/paths.h>
#include <util/sll/qtutil.h>
#include "../filepicker.h"

namespace LC
{
	bool ItemHandlerPath::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "path";
	}

	void ItemHandlerPath::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		FilePicker::Type type = FilePicker::Type::ExistingDirectory;
		if (item.attribute ("pickerType") == "openFileName")
			type = FilePicker::Type::OpenFileName;
		else if (item.attribute ("pickerType") == "saveFileName")
			type = FilePicker::Type::SaveFileName;

		FilePicker *picker = new FilePicker (type, XSD_->GetWidget ());
		const QVariant& value = XSD_->GetValue (item);
		picker->SetText (value.toString ());
		picker->setObjectName (item.attribute ("property"));
		if (item.attribute ("onCancel") == "clear")
			picker->SetClearOnCancel (true);
		if (item.hasAttribute ("filter"))
			picker->SetFilter (item.attribute ("filter"));

		connect (picker,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (updatePreferences ()));

		picker->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		picker->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0);
		lay->addWidget (picker, row, 1);
	}

	QVariant ItemHandlerPath::GetValue (const QDomElement& item, QVariant value) const
	{
		if (!value.toString ().isEmpty ())
			return value;

		if (item.attribute ("defaultHomePath") == "true")
			return QDir::homePath ();
		if (!item.hasAttribute ("default"))
			return {};

		static const QMap<QString, QString> str2loc
		{
			{ "DOCUMENTS", QStandardPaths::writableLocation (QStandardPaths::DocumentsLocation) },
			{ "DESKTOP", QStandardPaths::writableLocation (QStandardPaths::DesktopLocation) },
			{ "MUSIC", QStandardPaths::writableLocation (QStandardPaths::MusicLocation) },
			{ "MOVIES", QStandardPaths::writableLocation (QStandardPaths::MoviesLocation) },
			{ "LCDIR", Util::GetUserDir (Util::UserDir::LC, {}).absolutePath () },
			{ "CACHEDIR", Util::GetUserDir (Util::UserDir::Cache, {}).absolutePath () }
		};

		auto text = item.attribute ("default");
		for (const auto& pair : Util::Stlize (str2loc))
			if (text.startsWith ("{" + pair.first + "}"))
			{
				text.replace (0, pair.first.length () + 2, pair.second);
				break;
			}

		return text;
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

	QVariant ItemHandlerPath::GetObjectValue (QObject *object) const
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
}
