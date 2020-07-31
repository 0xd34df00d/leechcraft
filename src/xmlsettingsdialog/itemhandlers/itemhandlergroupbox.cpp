/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlergroupbox.h"
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QtDebug>

namespace LC
{
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
		box->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		box->setProperty ("SearchTerms", box->title ());

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

	QVariant ItemHandlerGroupbox::GetObjectValue (QObject *object) const
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
}
