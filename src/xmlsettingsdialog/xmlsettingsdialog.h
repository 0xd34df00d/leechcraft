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

/*
	Copyright (c) 2008-2009 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *																		 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	 *
 *   (at your option) any later version.								   *
 *																		 *
 ***************************************************************************
*/
#ifndef XMLSETTINGSDIALOG_XMLSETTINGSDIALOG_H
#define XMLSETTINGSDIALOG_XMLSETTINGSDIALOG_H
#include <QWidget>
#include <QString>
#include <QMap>
#include <QVariant>
#include <boost/shared_ptr.hpp>
#include "xsdconfig.h"

class QStackedWidget;
class QListWidget;
class QPushButton;
class QDomElement;
class QFormLayout;
class QDomDocument;

namespace LeechCraft
{
	class ItemHandlerFactory;

	namespace Util
	{
		class XmlSettingsDialog : public QWidget
		{
			Q_OBJECT

			QStackedWidget *Pages_;
			QStringList Titles_;
			QObject *WorkingObject_;
			QString DefaultLang_;
			boost::shared_ptr<QDomDocument> Document_;
			QList<QWidget*> Customs_;
			ItemHandlerFactory *HandlersManager_;
		public:
			struct LangElements
			{
				bool Valid_;
				QPair<bool, QString> Label_;
				QPair<bool, QString> Suffix_;
			};

			XMLSETTINGSMANAGER_API XmlSettingsDialog ();
			XMLSETTINGSMANAGER_API virtual ~XmlSettingsDialog ();
			XMLSETTINGSMANAGER_API void RegisterObject (QObject*, const QString&);

			/** @brief Returns the current XML.
			 *
			 * Returns the XML with the default settings set to current
			 * settings.
			 *
			 * @return String with the current XML.
			 */
			XMLSETTINGSMANAGER_API QString GetXml () const;

			/** @brief Sets the settings to XML's ones.
			 *
			 * Sets settings to defaults of the passed XML.
			 *
			 * @param[in] xml The XML to take data from.
			 */
			XMLSETTINGSMANAGER_API void MergeXml (const QByteArray& xml);

			/** @brief Sets custom widget mentioned in the XML.
			 *
			 * Sets the placeholder named name, mentioned in the XML, to
			 * contain the widget. If there is no name or more than one
			 * name, throws std::runtime_error. Widget's slots accept()
			 * and reject() would be called when the dialog is accepted
			 * or rejected.
			 *
			 * Default layout of the placeholder is QVBoxLayout, so if
			 * you call this function more than once with the same name
			 * but different widget parameters, it would create a nice
			 * vertical layout of all added widgets in the placeholder.
			 *
			 * It's your duty to do anything related to the widget,
			 * XmlSettingsDialog would only make some place for it and
			 * notify if the dialog is accepted or rejected.
			 *
			 * @param[in] name Name of the placeholder to replace.
			 * @param[in] widget The widget to add to the layout of the
			 * placeholder.
			 *
			 * @exception std::runtime_error If there is no or more than
			 * one name.
			 */
			XMLSETTINGSMANAGER_API void SetCustomWidget (const QString& name,
					QWidget *widget);

			/** @brief Sets the current page to page.
			 *
			 * @param[in] page Number of the page.
			 */
			XMLSETTINGSMANAGER_API void SetPage (int page);

			/** @brief Returns the list of all the pages.
			 *
			 * @return The names of the pages.
			 */
			XMLSETTINGSMANAGER_API QStringList GetPages () const;

			/** @brief Returns the human-readable label for the given
			 * element.
			 *
			 * Returns the proper label for an element according to the
			 * current locale and XML settings.
			 *
			 * @param[in] element The element for which label should be
			 * returned.
			 * @return The label suitable for showing to the user in the
			 * current language.
			 *
			 * @sa GetLangElements()
			 */
			QString GetLabel (const QDomElement& element) const;

			/** @brief Returns the current value for the given element.
			 *
			 * This function checks the object associated with this
			 * settings dialog and returns a value previously stored or
			 * default value if no previously set value exists.
			 *
			 * If ignoreObject is set, this function ignores the
			 * preferences already present and just parses the XML file.
			 * In this case, the default value is returned.
			 *
			 * @param[in] element The element for which the preferences
			 * value should be returned.
			 * @param[in] ignoreObject Whether associated object should
			 * be ignored.
			 * @return The current preferences value for the element.
			 */
			QVariant GetValue (const QDomElement& element,
					bool ignoreObject = false) const;

			/** @brief Returns the list of images associated with the
			 * given element.
			 *
			 * This function iterates over all children with name
			 * "binary" and creates the list of images that could be
			 * retreieved from those binary children.
			 *
			 * @param[in] element The element to collect images from.
			 * @return The list of images, if any.
			 */
			QList<QImage> GetImages (const QDomElement& element) const;

			/** @brief Parses the given element under the given parent
			 * widget.
			 *
			 * @param[in] element The element to inspect.
			 * @param[in] parent The parent widget under which to build
			 * up the representation.
			 */
			void ParseEntity (const QDomElement& element, QWidget *parent);

			/** @brief Get other human-readable messages from the
			 * element.
			 *
			 * This function is similar to GetLabel(), but instead it
			 * returns a slightly different list of user-visible
			 * elements.
			 *
			 * @param[in] element The element to inspect.
			 * @return The LangElements structure filled with available
			 * language/locale-dependent strings.
			 *
			 * @sa GetLabel()
			 */
			LangElements GetLangElements (const QDomElement& element) const;
		private:
			void HandleDeclaration (const QDomElement&);
			void ParsePage (const QDomElement&);
			void ParseItem (const QDomElement&, QWidget*);
			void UpdateXml (bool = false);
			void UpdateSingle (const QString&, const QVariant&, QDomElement&);
			void SetValue (QWidget*, const QVariant&);
		protected:
			bool eventFilter (QObject*, QEvent*);
		public slots:
			virtual void accept ();
			virtual void reject ();
		private slots:
			void handleCustomDestroyed ();
			void handlePushButtonReleased ();
		signals:
			XMLSETTINGSMANAGER_API void pushButtonClicked (const QString&);
		};
	};
};

#endif

