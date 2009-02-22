/*
    Copyright (c) 2008-2009 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
#ifndef XMLSETTINGSDIALOG_H
#define XMLSETTINGSDIALOG_H
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
			QListWidget *Sections_;
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
			LEECHCRAFT_API XmlSettingsDialog ();
			LEECHCRAFT_API virtual ~XmlSettingsDialog ();
			LEECHCRAFT_API void RegisterObject (QObject*, const QString&);

			/** @brief Returns the current XML.
			 *
			 * Returns the XML with the default settings set to current
			 * settings.
			 *
			 * @return String with the current XML.
			 */
			LEECHCRAFT_API QString GetXml () const;

			/** @brief Sets the settings to XML's ones.
			 *
			 * Sets settings to defaults of the passed XML.
			 *
			 * @param[in] xml The XML to take data from.
			 */
			LEECHCRAFT_API void MergeXml (const QByteArray& xml);

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
			LEECHCRAFT_API void SetCustomWidget (const QString& name,
					QWidget *widget);
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
			void DoPushButton (const QDomElement&, QFormLayout*);
			void DoCustomWidget (const QDomElement&, QFormLayout*);
			QList<QImage> GetImages (const QDomElement&) const;
			void UpdateXml (bool = false);
			void UpdateSingle (const QString&, const QVariant&, QDomElement&);
		public slots:
			virtual void accept ();
			virtual void reject ();
		private slots:
			void handleCustomDestroyed ();
			void updatePreferences ();
			void handlePushButtonReleased ();
		signals:
			LEECHCRAFT_API void pushButtonClicked (const QString&);
		};
	};
};

#endif

