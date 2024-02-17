/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerradio.h"
#include <QRadioButton>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include "../xmlsettingsdialog.h"
#include "../widgets/radiogroup.h"

namespace LC
{
	ItemRepresentation HandleRadio (const ItemContext& ctx)
	{
		const auto group = new RadioGroup { ctx.Label_ };

		QVariant defOption;

		QStringList searchTerms {};
		for (const auto& option : Util::DomChildren (ctx.Elem_, "option"_qs))
		{
			const auto& optLabel = ctx.XSD_.GetLabel (option);
			searchTerms << optLabel;
			const auto& optDescr = ctx.XSD_.GetDescription (option);

			const auto& name = option.attribute ("name"_qs);

			const auto isDefault = option.attribute ("default"_qs) == "true"_ql;
			if (isDefault || defOption.isNull ())
				defOption = name;
			group->AddButton (name, optLabel, optDescr, isDefault);
		}

		SetChangedSignal (ctx, group, &RadioGroup::valueChanged);

		return
		{
			.Widget_ = group,
			.SearchTerms_ = searchTerms,

			.DefaultValue_ = defOption,
			.Getter_ = [group] { return group->GetValue (); },
			.Setter_ = [group] (const QVariant& value) { group->SetValue (value.toString ()); },
		};
	}
}
