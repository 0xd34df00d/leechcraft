/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerspinbox.h"
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <util/sll/qtutil.h>
#include "../xmlsettingsdialog.h"

namespace LC
{
	namespace
	{
		std::optional<QString> GetAttr (const QDomElement& item, std::initializer_list<QString> attrNames)
		{
			for (const auto& attr : attrNames)
				if (item.hasAttribute (attr))
					return item.attribute (attr);

			return {};
		}

		template<typename WidgetType>
		ItemRepresentation HandleSpinboxBase (const ItemContext& ctx, auto cvt)
		{
			using ValueType = decltype (cvt (QString {}));

			const auto& item = ctx.Elem_;

			const auto box = new WidgetType {};
			if (const auto min = GetAttr (item, { "minimum"_qs, "min"_qs }))
				box->setMinimum (cvt (*min));
			if (const auto max = GetAttr (item, { "maximum"_qs, "max"_qs }))
				box->setMaximum (cvt (*max));
			if (const auto suffix = GetAttr (item, { "suffix"_qs, "suf"_qs }))
				box->setSuffix (*suffix);
			if (item.hasAttribute ("step"_qs))
				box->setSingleStep (cvt (item.attribute ("step"_qs)));

			const auto& langs = ctx.XSD_.GetLangElements (item);
			if (langs.Suffix_)
				box->setSuffix (*langs.Suffix_);
			if (langs.SpecialValue_)
				box->setSpecialValueText (*langs.SpecialValue_);

			SetChangedSignal (ctx, box, &WidgetType::valueChanged);

			return
			{
				.Widget_ = box,

				.DefaultValue_ = cvt (item.attribute ("default"_qs)),
				.Getter_ = [box] { return box->value (); },
				.Setter_ = [box] (const QVariant& value) { box->setValue (value.value<ValueType> ()); },
			};
		}
	}

	ItemRepresentation HandleSpinbox (const ItemContext& ctx)
	{
		return HandleSpinboxBase<QSpinBox> (ctx, +[] (const QString& str) { return str.toInt (); });
	}

	ItemRepresentation HandleSpinboxDouble (const ItemContext& ctx)
	{
		return HandleSpinboxBase<QDoubleSpinBox> (ctx, +[] (const QString& str) { return str.toDouble (); });
	}
}
