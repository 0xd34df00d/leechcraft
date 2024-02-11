/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerspinboxbase.h"
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "../xmlsettingsdialog.h"

namespace LC
{
	template<typename WidgetType, typename ValueType>
	ItemHandlerSpinboxBase<WidgetType, ValueType>::ItemHandlerSpinboxBase (Converter_t cvt, const QString& etype, Util::XmlSettingsDialog *xsd)
	: ItemHandlerStringGetValue { xsd }
	, Converter_ { cvt }
	, ElementType_ { etype }
	{
	}

	template<typename WidgetType, typename ValueType>
	bool ItemHandlerSpinboxBase<WidgetType, ValueType>::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type"_qs) == ElementType_;
	}

	template<typename WidgetType, typename ValueType>
	void ItemHandlerSpinboxBase<WidgetType, ValueType>::Handle (const QDomElement& item, QWidget *pwidget)
	{
		const auto lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		const auto label = new QLabel { XSD_->GetLabel (item) };
		label->setWordWrap (false);

		const auto box = new WidgetType { XSD_->GetWidget () };
		XSD_->SetTooltip (box, item);
		box->setObjectName (item.attribute ("property"_qs));

		if (item.hasAttribute ("minimum"_qs))
			box->setMinimum (Converter_ (item.attribute ("minimum"_qs)));
		if (item.hasAttribute ("maximum"_qs))
			box->setMaximum (Converter_ (item.attribute ("maximum"_qs)));
		if (item.hasAttribute ("step"_qs))
			box->setSingleStep (Converter_ (item.attribute ("step"_qs)));
		if (item.hasAttribute ("suffix"_qs))
			box->setSuffix (item.attribute ("suffix"_qs));

		const auto& langs = XSD_->GetLangElements (item);
		if (langs.Label_)
			label->setText (*langs.Label_);
		if (langs.Suffix_)
			box->setSuffix (*langs.Suffix_);
		if (langs.SpecialValue_)
			box->setSpecialValueText (*langs.SpecialValue_);

		box->setValue (XSD_->GetValue (item).value<ValueType> ());
		connect (box,
				&WidgetType::valueChanged,
				this,
				&ItemHandlerSpinboxBase::updatePreferences);

		box->setProperty ("ItemHandler", QVariant::fromValue<QObject*> (this));
		box->setProperty ("SearchTerms", label->text ());

		const int row = lay->rowCount ();
		lay->addWidget (label, row, 0, Qt::AlignRight);
		lay->addWidget (box, row, 1);
	}

	template<typename WidgetType, typename ValueType>
	void ItemHandlerSpinboxBase<WidgetType, ValueType>::SetValue (QWidget *widget, const QVariant& value) const
	{
		if (auto spinbox = qobject_cast<WidgetType*> (widget))
			spinbox->setValue (value.value<ValueType> ());
		else
			qWarning () << Q_FUNC_INFO
					<< "not an expected class"
					<< widget;
	}

	template<typename WidgetType, typename ValueType>
	QVariant ItemHandlerSpinboxBase<WidgetType, ValueType>::GetObjectValue (QObject *object) const
	{
		if (auto spinbox = qobject_cast<WidgetType*> (object))
			return spinbox->value ();

		qWarning () << Q_FUNC_INFO
				<< "not an expected class"
				<< object;
		return QVariant ();
	}

	template class ItemHandlerSpinboxBase<QSpinBox, int>;
	template class ItemHandlerSpinboxBase<QDoubleSpinBox, double>;
}
