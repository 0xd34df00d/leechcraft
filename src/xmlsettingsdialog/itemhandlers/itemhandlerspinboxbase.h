/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemhandlerstringgetvalue.h"
#include <functional>
#include <QGridLayout>
#include <QLabel>
#include <QtDebug>
#include "../xmlsettingsdialog.h"

namespace LC
{
	template<typename WidgetType, typename ValueType>
	class ItemHandlerSpinboxBase : public ItemHandlerStringGetValue
	{
	public:
		using Converter_t = std::function<ValueType (QString)>;
	private:
		Converter_t Converter_;
		QString ElementType_;
		const char *ChangedSignal_;
	public:
		ItemHandlerSpinboxBase (Converter_t cvt, const QString& etype, const char *cs, Util::XmlSettingsDialog *xsd)
		: ItemHandlerStringGetValue { xsd }
		, Converter_ { cvt }
		, ElementType_ { etype }
		, ChangedSignal_ { cs }
		{
		}

		bool CanHandle (const QDomElement& element) const
		{
			return element.attribute ("type") == ElementType_;
		}

		void Handle (const QDomElement& item, QWidget *pwidget)
		{
			QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
			QLabel *label = new QLabel (XSD_->GetLabel (item));
			label->setWordWrap (false);

			WidgetType *box = new WidgetType (XSD_->GetWidget ());
			XSD_->SetTooltip (box, item);
			box->setObjectName (item.attribute ("property"));

			if (item.hasAttribute ("minimum"))
				box->setMinimum (Converter_ (item.attribute ("minimum")));
			if (item.hasAttribute ("maximum"))
				box->setMaximum (Converter_ (item.attribute ("maximum")));
			if (item.hasAttribute ("step"))
				box->setSingleStep (Converter_ (item.attribute ("step")));
			if (item.hasAttribute ("suffix"))
				box->setSuffix (item.attribute ("suffix"));

			const auto& langs = XSD_->GetLangElements (item);
			if (langs.Label_)
				label->setText (*langs.Label_);
			if (langs.Suffix_)
				box->setSuffix (*langs.Suffix_);
			if (langs.SpecialValue_)
				box->setSpecialValueText (*langs.SpecialValue_);

			QVariant value = XSD_->GetValue (item);

			box->setValue (value.value<ValueType> ());
			connect (box,
					ChangedSignal_,
					this,
					SLOT (updatePreferences ()));

			box->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
			box->setProperty ("SearchTerms", label->text ());

			int row = lay->rowCount ();
			lay->setColumnMinimumWidth (0, 10);
			lay->setColumnStretch (0, 1);
			lay->setColumnStretch (1, 5);
			lay->addWidget (label, row, 0, Qt::AlignRight);
			lay->addWidget (box, row, 1);
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
		QVariant GetObjectValue (QObject *object) const
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
}
