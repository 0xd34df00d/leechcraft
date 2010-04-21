/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "itemhandlerfactory.h"
#include <QWidget>
#include <QtDebug>
#include "itemhandlers/itemhandlerspinboxrange.h"
#include "itemhandlers/itemhandlerpath.h"
#include "itemhandlers/itemhandlerfont.h"
#include "itemhandlers/itemhandlercolor.h"
#include "itemhandlers/itemhandlercheckbox.h"
#include "itemhandlers/itemhandlergroupbox.h"
#include "itemhandlers/itemhandlerradio.h"
#include "itemhandlers/itemhandlercombobox.h"
#include "itemhandlers/itemhandlerlineedit.h"
#include "itemhandlers/itemhandlerspinboxbase.h"
#include "itemhandlers/itemhandlerspinboxdouble.h"
#include "itemhandlers/itemhandlerspinbox.h"
#include "itemhandlers/itemhandlerpushbutton.h"
#include "itemhandlers/itemhandlercustomwidget.h"

namespace LeechCraft
{
	ItemHandlerFactory::ItemHandlerFactory ()
	{
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerCheckbox ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerLineEdit ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerGroupbox ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerSpinbox ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerSpinboxDouble ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerRadio ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerCombobox ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerSpinboxRange ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerPushButton ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerCustomWidget ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerPath ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerFont ());
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerColor ());
	}

	ItemHandlerFactory::~ItemHandlerFactory ()
	{
	}

	bool ItemHandlerFactory::Handle (const QDomElement& element,
			QWidget* widget)
	{
		Q_FOREACH (ItemHandlerBase_ptr handler, Handlers_)
			if (handler->CanHandle (element))
			{
				handler->Handle (element, widget);
				return true;
			}

		qWarning () << Q_FUNC_INFO
			<< "unhandled element of type"
			<< element.attribute ("type");

		return false;
	}

	void ItemHandlerFactory::SetValue (QWidget *widget,
			const QVariant& value) const
	{
		QObject *object = widget->
				property ("ItemHandler").value<QObject*> ();
		if (!object)
		{
			qWarning () << Q_FUNC_INFO
				<< "ItemHandler propery for widget is not set"
				<< widget;
			return;
		}
		ItemHandlerBase *base =
				static_cast<ItemHandlerBase*> (object);
		base->SetValue (widget, value);
	}

	bool ItemHandlerFactory::UpdateSingle (QDomElement& element,
			const QVariant& value) const
	{
		Q_FOREACH (ItemHandlerBase_ptr handler, Handlers_)
			if (handler->CanHandle (element))
			{
				handler->UpdateValue (element, value);
				return true;
			}

		return false;
	}

	QVariant ItemHandlerFactory::GetValue (const QDomElement& element,
			const QVariant& value) const
	{
		Q_FOREACH (ItemHandlerBase_ptr handler, Handlers_)
			if (handler->CanHandle (element))
				return handler->GetValue (element, value);
		return QVariant ();
	}

	ItemHandlerBase::Prop2NewValue_t ItemHandlerFactory::GetNewValues () const
	{
		ItemHandlerBase::Prop2NewValue_t result;
		Q_FOREACH (ItemHandlerBase_ptr handler, Handlers_)
			result.unite (handler->GetChangedProperties ());
		return result;
	}

	void ItemHandlerFactory::ClearNewValues ()
	{
		Q_FOREACH (ItemHandlerBase_ptr handler, Handlers_)
			handler->ClearChangedProperties ();
	}
};
