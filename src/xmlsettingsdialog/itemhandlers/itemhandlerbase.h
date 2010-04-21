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

#ifndef XMLSETTINGSDIALOG_ITEMHANDLERS_ITEMHANDLERBASE_H
#define XMLSETTINGSDIALOG_ITEMHANDLERS_ITEMHANDLERBASE_H
#include <boost/shared_ptr.hpp>
#include <QMap>
#include <QVariant>
#include <QDomElement>
#include "../xmlsettingsdialog.h"

namespace LeechCraft
{
	/** Base class for the handlers of the <item />-QDomElements.
	 */
	class ItemHandlerBase : public QObject
	{
		Q_OBJECT
	public:
		typedef QMap<QString, QVariant> Prop2NewValue_t;

		static void SetXmlSettingsDialog (Util::XmlSettingsDialog *xsd);

		ItemHandlerBase ();
		virtual ~ItemHandlerBase ();

		/** @brief Whether this item handler can handle this particular
		 * element.
		 *
		 * @param[in] element The element to try to handle.
		 * @return Whether this element can be handled.
		 */
		virtual bool CanHandle (const QDomElement& element) const = 0;

		/** @brief Creates the representation widget for the given
		 * element and parent widget pwidget.
		 *
		 * The created widget's property "ItemHandler" should point to
		 * the class that created in in order to retrieve it later.
		 *
		 * pwidget is guaranteed to have a layout, and the layout is
		 * QFormLayout.
		 *
		 * @param[in] element The element to make representation for.
		 * @param[in] pwidget The parent widget of the representation
		 * widget.
		 */
		virtual void Handle (const QDomElement& element, QWidget *pwidget) = 0;

		/** @brief Return the value of the given element and given
		 * predefined value.
		 *
		 * This function should inspect the given element and return
		 * a QVariant containing its value. If value is given and it is
		 * valid and sensible for this widget, it should be considered
		 * instead.
		 *
		 * @param[in] element The element to retrieve value for.
		 * @param[in] value The default value that should be considered
		 * just in case.
		 * @return The resulting value.
		 */
		virtual QVariant GetValue (const QDomElement& element,
				QVariant value) const = 0;

		/** @brief Sets the value for the widget created earlier.
		 *
		 * @param[in] widget The widget created earlier by the call to
		 * Handle().
		 * @param[in] value The value that should be set for this
		 * widget.
		 */
		virtual void SetValue (QWidget *widget, const QVariant& value) const = 0;

		/** @brief Update the value of the given element.
		 *
		 * This function should update the current default value of the
		 * given element to a new value.
		 *
		 * @param[in,out] element
		 * @param[in] value The new value for this element.
		 */
		virtual void UpdateValue (QDomElement& element, const QVariant& value) const = 0;

		/** @brief Returns the list of the changed properties for
		 * widgets that are managed by this item handler.
		 *
		 * @return The list of the changed properties and their values.
		 */
		virtual Prop2NewValue_t GetChangedProperties () const;

		/** @brief Clears the list of the changed properties for
		 * widgets that are managed by this item handler.
		 */
		virtual void ClearChangedProperties ();
	protected:
		/** @brief Returns the value of the object.
		 *
		 * This function should return a correct value, assuming that
		 * object is really a pointer to a widget created earlier in
		 * Handle() function by this item handler. So it's safe to
		 * qobject_cast.
		 *
		 * @param[in] object Pointer to a widget previously created by
		 * this item handler.
		 * @return The value that the widget pointed by object holds.
		 *
		 * @sa Handle()
		 */
		virtual QVariant GetValue (QObject *object) const = 0;

		Prop2NewValue_t ChangedProperties_;
		static Util::XmlSettingsDialog *XSD_;
	protected slots:
		virtual void updatePreferences ();
	};

	typedef boost::shared_ptr<ItemHandlerBase> ItemHandlerBase_ptr;
};

#endif
