/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerradio.h"
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QtDebug>
#include "../radiogroup.h"

namespace LC
{
	bool ItemHandlerRadio::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "radio";
	}

	void ItemHandlerRadio::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		RadioGroup *group = new RadioGroup (XSD_->GetWidget ());
		group->setObjectName (item.attribute ("property"));

		QStringList searchTerms;
		QDomElement option = item.firstChildElement ("option");
		while (!option.isNull ())
		{
			QRadioButton *button = new QRadioButton (XSD_->GetLabel (option));
			searchTerms << button->text ();
			XSD_->SetTooltip (button, option);
			button->setObjectName (option.attribute ("name"));
			group->AddButton (button,
					option.hasAttribute ("default") &&
					option.attribute ("default") == "true");
			option = option.nextSiblingElement ("option");
		}

		QVariant value = XSD_->GetValue (item);

		connect (group,
				SIGNAL (valueChanged ()),
				this,
				SLOT (updatePreferences ()));

		QGroupBox *box = new QGroupBox (XSD_->GetLabel (item));
		QVBoxLayout *layout = new QVBoxLayout ();
		box->setLayout (layout);
		layout->addWidget (group);

		searchTerms << box->title ();
		group->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		group->setProperty ("SearchTerms", searchTerms);

		lay->addWidget (box, lay->rowCount (), 0);
	}

	void ItemHandlerRadio::SetValue (QWidget *widget, const QVariant& value) const
	{
		RadioGroup *radiogroup = qobject_cast<RadioGroup*> (widget);
		if (!radiogroup)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a RadioGroup"
				<< widget;
			return;
		}
		radiogroup->SetValue (value.toString ());
	}

	QVariant ItemHandlerRadio::GetObjectValue (QObject *object) const
	{
		RadioGroup *radiogroup = qobject_cast<RadioGroup*> (object);
		if (!radiogroup)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a RadioGroup"
				<< object;
			return QVariant ();
		}
		return radiogroup->GetValue ();
	}
}
