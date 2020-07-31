/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerspinboxrange.h"
#include <QLabel>
#include <QGridLayout>
#include <QtDebug>
#include "../rangewidget.h"

namespace LC
{
	bool ItemHandlerSpinboxRange::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "spinboxrange";
	}

	void ItemHandlerSpinboxRange::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);

		RangeWidget *widget = new RangeWidget ();
		XSD_->SetTooltip (widget, item);
		widget->setObjectName (item.attribute ("property"));
		widget->SetMinimum (item.attribute ("minimum").toInt ());
		widget->SetMaximum (item.attribute ("maximum").toInt ());

		const QVariant& value = XSD_->GetValue (item);

		widget->SetRange (value);
		connect (widget,
				SIGNAL (changed ()),
				this,
				SLOT (updatePreferences ()));

		widget->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		widget->setProperty ("SearchTerms", label->text ());

		int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignRight);
		lay->addWidget (widget, row, 1);
	}

	QVariant ItemHandlerSpinboxRange::GetValue (const QDomElement& item,
			QVariant value) const
	{
		if (!value.isValid () ||
				value.isNull () ||
				!value.canConvert<QList<QVariant>> ())
		{
			const QStringList& parts = item.attribute ("default").split (":");
			QList<QVariant> result;
			if (parts.size () != 2)
			{
				qWarning () << "spinboxrange parse error, wrong default value";
				return QVariant ();
			}
			result << parts.at (0).toInt () << parts.at (1).toInt ();
			value = result;
		}
		return value;
	}

	void ItemHandlerSpinboxRange::SetValue (QWidget *widget,
			const QVariant& value) const
	{
		RangeWidget *rw = qobject_cast<RangeWidget*> (widget);
		if (!rw)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a RangeWidget"
				<< widget;
			return;
		}

		rw->SetRange (value);
	}

	void ItemHandlerSpinboxRange::UpdateValue (QDomElement& element,
			const QVariant& value) const
	{
		const QStringList& vals = value.toStringList ();
		if (vals.size () != 2)
		{
			qWarning () << Q_FUNC_INFO
				<< "spinboxrange value error, not 2 elems in list"
				<< value;
			return;
		}
		element.setAttribute ("default", vals.at (0) + ':' + vals.at (1));
	}

	QVariant ItemHandlerSpinboxRange::GetObjectValue (QObject *object) const
	{
		RangeWidget *widget = qobject_cast<RangeWidget*> (object);
		if (!widget)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a RangeWidget"
				<< object;
			return QVariant ();
		}
		return widget->GetRange ();
	}
}
