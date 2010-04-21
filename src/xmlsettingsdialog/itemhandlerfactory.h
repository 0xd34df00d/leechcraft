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

#ifndef XMLSETTINGSDIALOG_ITEMHANDLERFACTORY_H
#define XMLSETTINGSDIALOG_ITEMHANDLERFACTORY_H
#include <QList>
#include <QDomElement>
#include "itemhandlers/itemhandlerbase.h"

namespace LeechCraft
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
		ItemHandlerFactory ();
		~ItemHandlerFactory ();

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
		 * have QFormLayout as layout() set.
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
	};
};

#endif
