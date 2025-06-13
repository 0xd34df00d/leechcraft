/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlercheckbox.h"
#include <QCheckBox>
#include "defaultvaluegetters.h"

namespace LC
{
	ItemRepresentation HandleCheckbox (const ItemContext& ctx)
	{
		const auto box = new QCheckBox { ctx.Label_ };
		SetChangedSignal (ctx, box, &QCheckBox::checkStateChanged);
		return
		{
			.Widget_ = box,
			.LabelPosition_ = LabelPosition::None,

			.DefaultValue_ = GetDefaultBooleanValue (ctx.Default_),
			.Getter_ = [box] { return box->checkState () == Qt::Checked; },
			.Setter_ = [box] (const QVariant& val) { box->setCheckState (val.toBool () ? Qt::Checked : Qt::Unchecked); },
		};
	}
}
