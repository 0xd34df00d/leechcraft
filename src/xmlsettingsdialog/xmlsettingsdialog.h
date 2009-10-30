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
#include "config.h"

class QStackedWidget;
class QListWidget;
class QPushButton;
class QDomElement;
class QFormLayout;
class QDomDocument;

namespace LeechCraft
{
	namespace Util
	{
		class XmlSettingsDialog : public QWidget
		{
			Q_OBJECT

			QStackedWidget *Pages_;
			QStringList Titles_;
			QObject *WorkingObject_;
			typedef QMap<QString, QVariant> Property2Value_t;
			Property2Value_t Prop2NewValue_;
			QString DefaultLang_;
			boost::shared_ptr<QDomDocument> Document_;
			struct LangElements
			{
				bool Valid_;
				QPair<bool, QString> Label_;
				QPair<bool, QString> Suffix_;
			};
			QList<QWidget*> Customs_;
		public:
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
		private:
			void HandleDeclaration (const QDomElement&);
			void ParsePage (const QDomElement&);
			void ParseEntity (const QDomElement&, QWidget*);
			void ParseItem (const QDomElement&, QWidget*);
			QString GetLabel (const QDomElement&) const;
			LangElements GetLangElements (const QDomElement&) const;
			QVariant GetValue (const QDomElement&, bool = false) const;
			void DoLineedit (const QDomElement&, QFormLayout*);
			void DoCheckbox (const QDomElement&, QFormLayout*);
			void DoSpinbox (const QDomElement&, QFormLayout*);
			void DoDoubleSpinbox (const QDomElement&, QFormLayout*);
			void DoGroupbox (const QDomElement&, QFormLayout*);
			void DoSpinboxRange (const QDomElement&, QFormLayout*);
			void DoPath (const QDomElement&, QFormLayout*);
			void DoRadio (const QDomElement&, QFormLayout*);
			void DoCombobox (const QDomElement&, QFormLayout*);
			void DoFont (const QDomElement&, QFormLayout*);
			void DoColor (const QDomElement&, QFormLayout*);
			void DoPushButton (const QDomElement&, QFormLayout*);
			void DoCustomWidget (const QDomElement&, QFormLayout*);
			QList<QImage> GetImages (const QDomElement&) const;
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
			void updatePreferences ();
			void handlePushButtonReleased ();
		signals:
			XMLSETTINGSMANAGER_API void pushButtonClicked (const QString&);
		};
	};
};

#endif

