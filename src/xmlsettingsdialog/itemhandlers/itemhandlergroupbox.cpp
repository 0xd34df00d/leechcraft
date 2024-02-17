/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlergroupbox.h"
#include <QGroupBox>
#include <QFormLayout>
#include "../xmlsettingsdialog.h"
#include "defaultvaluegetters.h"

namespace LC
{
	ItemRepresentation HandleGroupbox (const ItemContext& ctx)
	{
		const auto box = new QGroupBox { ctx.Label_ };
		box->setCheckable (true);
		SetChangedSignal (ctx, box, &QGroupBox::toggled);

		const auto groupLayout = new QFormLayout;
		groupLayout->setContentsMargins (2, 2, 2, 2);
		box->setLayout (groupLayout);

		ctx.XSD_.ParseEntity (ctx.Elem_, *groupLayout);

		return
		{
			.Widget_ = box,
			.LabelPosition_ = LabelPosition::None,

			.DefaultValue_ = GetDefaultBooleanValue (ctx.Elem_),
			.Getter_ = [box] { return box->isChecked (); },
			.Setter_ = [box] (const QVariant& value) { box->setChecked (value.toBool ()); },
		};
	}
}
