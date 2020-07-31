/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QList>
#include <QDomElement>
#include <QHash>
#include "itemhandlers/itemhandlerbase.h"

namespace LC
{
	/** @brief Manager for the item handlers.
	 *
	 * This class manages the handlers for various items appearing in
	 * the XML Settings Dialogs.
	 *
	 * @sa ItemHandlerBase
	 */
	class ItemHandlerFactory
	{
		QList<ItemHandlerBase_ptr> Handlers_;
	public:
		using DataSourceSetter_t = std::function<void (QString, QAbstractItemModel*, Util::XmlSettingsDialog*)>;
	private:
		QHash<QString, DataSourceSetter_t> Propname2DataSourceSetter_;
	public:
		ItemHandlerFactory (Util::XmlSettingsDialog*);

		/** @brief Create a visual representation for the given element
		 * with the given parent widget.
		 *
		 * This function finds a suitable Item Handler that can handle
		 * the given element and calls its ItemHandlerBase::Handle
		 * function in order to create the visual representation of the
		 * element.
		 *
		 * When laying out the widgets in the visual representation,
		 * widget should be used as a parent widget. It's guaranteed to
		 * have QGridLayout as layout() set.
		 *
		 * The widget that is named after property name (defined in the
		 * element) would be used later in SetValue().
		 *
		 * @param[in] element The element from the settings.
		 * @param[in] widget The parent widget of the visual
		 * representation.
		 * @return true if the element was handled successfully, false
		 * otherwise.
		 */
		bool Handle (const QDomElement& element, QWidget *widget);

		/** @brief Set the given value for the given widget created
		 * previously with some Item Handler in Handle().
		 *
		 * The widget is created earlier by an Item Handler inside the
		 * Handle() function. It's the widget that has a property
		 * corresponding to the property name from the element parameter
		 * of @Handle()@.
		 *
		 * @param[in,out] widget The widget created earlier inside
		 * Handle().
		 * @param[in] value The new value for this widget.
		 */
		void SetValue (QWidget *widget, const QVariant& value) const;
		bool UpdateSingle (QDomElement& element, const QVariant& value) const;
		QVariant GetValue (const QDomElement& element, const QVariant& value) const;

		/** @brief Returns the list of all changed properties.
		 *
		 * Returns the list of all changed properties since
		 * initialization or previous calls to ClearNewValues().
		 *
		 * @return The list of changed properties.
		 *
		 * @sa ClearNewValues()
		 */
		ItemHandlerBase::Prop2NewValue_t GetNewValues () const;

		/** @brief Clear the list of changed properties.
		 *
		 * Clears the list of changed properties. This is used, for
		 * example, when accepting or rejecting changes by the dialog.
		 *
		 * @sa GetNewValues()
		 */
		void ClearNewValues ();

		/** @brief Set the datasource for the given property.
		 *
		 * @sa XmlSettingsDialog::SetDataSource()
		 */
		void SetDataSource (const QString&, QAbstractItemModel*, Util::XmlSettingsDialog*);

		void RegisterDatasourceSetter (const QString&, DataSourceSetter_t);
	};
}
