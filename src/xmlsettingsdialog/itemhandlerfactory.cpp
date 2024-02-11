/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
#include "itemhandlers/itemhandlermultiline.h"
#include "itemhandlers/itemhandlerspinboxbase.h"
#include "itemhandlers/itemhandlerspinboxdouble.h"
#include "itemhandlers/itemhandlerspinbox.h"
#include "itemhandlers/itemhandlerpushbutton.h"
#include "itemhandlers/itemhandlercustomwidget.h"
#include "itemhandlers/itemhandlerdataview.h"
#include "itemhandlers/itemhandlerlistview.h"
#include "itemhandlers/itemhandlertreeview.h"

namespace LC
{
	ItemHandlerFactory::ItemHandlerFactory (Util::XmlSettingsDialog *xsd)
	{
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerCheckbox (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerLineEdit (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerMultiLine (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerGroupbox (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerSpinbox (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerSpinboxDouble (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerRadio (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerCombobox (this, xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerSpinboxRange (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerPushButton (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerCustomWidget (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerPath (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerFont (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerColor (xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerDataView (this, xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerListView (this, xsd));
		Handlers_ << ItemHandlerBase_ptr (new ItemHandlerTreeView (this, xsd));
	}

	bool ItemHandlerFactory::Handle (const QDomElement& element,
			QWidget* widget)
	{
		for (const auto& handler : Handlers_)
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
				<< "ItemHandler property for widget is not set"
				<< widget;
			return;
		}
		ItemHandlerBase *base =
				static_cast<ItemHandlerBase*> (object);
		base->SetValue (widget, value);
	}

	QVariant ItemHandlerFactory::GetValue (const QDomElement& element,
			const QVariant& value) const
	{
		for (const auto& handler : Handlers_)
			if (handler->CanHandle (element))
				return handler->GetValue (element, value);
		return QVariant ();
	}

	ItemHandlerBase::Prop2NewValue_t ItemHandlerFactory::GetNewValues () const
	{
		ItemHandlerBase::Prop2NewValue_t result;
		for (const auto& handler : Handlers_)
			result.insert (handler->GetChangedProperties ());
		return result;
	}

	void ItemHandlerFactory::ClearNewValues ()
	{
		for (const auto& handler : Handlers_)
			handler->ClearChangedProperties ();
	}

	void ItemHandlerFactory::SetDataSource (const QString& property,
			QAbstractItemModel *model, Util::XmlSettingsDialog *xsd)
	{
		if (!Propname2DataSourceSetter_.contains (property))
		{
			qWarning () << Q_FUNC_INFO
					<< "there is no such registered datasource setter for property"
					<< property
					<< "; registered datasources:"
					<< Propname2DataSourceSetter_.keys ();
			return;
		}

		Propname2DataSourceSetter_ [property] (property, model, xsd);
	}

	void ItemHandlerFactory::RegisterDatasourceSetter (const QString& prop, ItemHandlerFactory::DataSourceSetter_t setter)
	{
		Propname2DataSourceSetter_ [prop] = setter;
	}
}
