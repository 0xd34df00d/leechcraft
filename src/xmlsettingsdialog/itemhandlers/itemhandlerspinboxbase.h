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

#ifndef XMLSETTINGSDIALOG_ITEMHANDLERS_ITEMHANDLERSPINBOXBASE_H
#define XMLSETTINGSDIALOG_ITEMHANDLERS_ITEMHANDLERSPINBOXBASE_H

#include "itemhandlerstringgetvalue.h"
#include <boost/function.hpp>
#include <QFormLayout>
#include <QLabel>
#include <QtDebug>
#include "../xmlsettingsdialog.h"

namespace LeechCraft
{
	template<typename WidgetType,
		typename ValueType>
	class ItemHandlerSpinboxBase : public ItemHandlerStringGetValue
	{
	public:
		typedef boost::function<ValueType (QString)> Converter_t;
	private:
		Converter_t Converter_;
		QString ElementType_;
		const char *ChangedSignal_;
	public:
		ItemHandlerSpinboxBase (Converter_t cvt, const QString& etype, const char *cs)
		: Converter_ (cvt)
		, ElementType_ (etype)
		, ChangedSignal_ (cs)
		{
		}

		virtual ~ItemHandlerSpinboxBase ()
		{
		}

		bool CanHandle (const QDomElement& element) const
		{
			return element.attribute ("type") == ElementType_;
		}

		void Handle (const QDomElement& item, QWidget *pwidget)
		{
			QFormLayout *lay = qobject_cast<QFormLayout*> (pwidget->layout ());
			QLabel *label = new QLabel (XSD_->GetLabel (item));
			label->setWordWrap (false);
			WidgetType *box = new WidgetType (XSD_);
			box->setObjectName (item.attribute ("property"));
			if (item.hasAttribute ("minimum"))
				box->setMinimum (Converter_ (item.attribute ("minimum")));
			if (item.hasAttribute ("maximum"))
				box->setMaximum (Converter_ (item.attribute ("maximum")));
			if (item.hasAttribute ("step"))
				box->setSingleStep (Converter_ (item.attribute ("step")));
			if (item.hasAttribute ("suffix"))
				box->setSuffix (item.attribute ("suffix"));
			Util::XmlSettingsDialog::LangElements langs = XSD_->GetLangElements (item);
			if (langs.Valid_)
			{
				if (langs.Label_.first)
					label->setText (langs.Label_.second);
				if (langs.Suffix_.first)
					box->setSuffix (langs.Suffix_.second);
			}

			QVariant value = XSD_->GetValue (item);

			box->setValue (value.value<ValueType> ());
			connect (box,
					ChangedSignal_,
					this,
					SLOT (updatePreferences ()));

			box->setProperty ("ItemHandler",
					QVariant::fromValue<QObject*> (this));

			lay->addRow (label, box);
		}

		void SetValue (QWidget *widget,
					const QVariant& value) const
		{
			WidgetType *spinbox = qobject_cast<WidgetType*> (widget);
			if (!spinbox)
			{
				qWarning () << Q_FUNC_INFO
					<< "not an expected class"
					<< widget;
				return;
			}
			spinbox->setValue (value.value<ValueType> ());
		}
	protected:
		QVariant GetValue (QObject *object) const
		{
			WidgetType *spinbox = qobject_cast<WidgetType*> (object);
			if (!spinbox)
			{
				qWarning () << Q_FUNC_INFO
					<< "not an expected class"
					<< object;
				return QVariant ();
			}
			return spinbox->value ();
		}
	};
};

#endif
