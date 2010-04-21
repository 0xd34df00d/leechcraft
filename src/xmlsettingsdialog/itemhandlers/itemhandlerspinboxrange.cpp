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
#include "itemhandlerspinboxrange.h"
#include <QLabel>
#include <QFormLayout>
#include <QtDebug>
#include "../rangewidget.h"

namespace LeechCraft
{
	ItemHandlerSpinboxRange::ItemHandlerSpinboxRange ()
	{
	}

	ItemHandlerSpinboxRange::~ItemHandlerSpinboxRange ()
	{
	}

	bool ItemHandlerSpinboxRange::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "spinboxrange";
	}

	void ItemHandlerSpinboxRange::Handle (const QDomElement& item,
			QWidget *pwidget)
	{
		QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
		QLabel *label = new QLabel (XSD_->GetLabel (item));
		label->setWordWrap (false);
		RangeWidget *widget = new RangeWidget ();
		widget->setObjectName (item.attribute ("property"));
		widget->SetMinimum (item.attribute ("minimum").toInt ());
		widget->SetMaximum (item.attribute ("maximum").toInt ());

		QVariant value = XSD_->GetValue (item);

		widget->SetRange (value);
		connect (widget,
				SIGNAL (changed ()),
				this,
				SLOT (updatePreferences ()));

		widget->setProperty ("ItemHandler",
				QVariant::fromValue<QObject*> (this));

		lay->addRow (label, widget);
	}

	QVariant ItemHandlerSpinboxRange::GetValue (const QDomElement& item,
			QVariant value) const
	{
		if (!value.isValid () ||
				value.isNull () ||
				!value.canConvert<QList<QVariant> > ())
		{
			QStringList parts = item.attribute ("default").split (":");
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
		QStringList vals = value.toStringList ();
		if (vals.size () != 2)
		{
			qWarning () << Q_FUNC_INFO
				<< "spinboxrange value error, not 2 elems in list"
				<< value;
			return;
		}
		element.setAttribute ("default", vals.at (0) + ':' + vals.at (1));
	}

	QVariant ItemHandlerSpinboxRange::GetValue (QObject *object) const
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
};
