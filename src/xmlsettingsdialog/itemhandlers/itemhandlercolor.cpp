/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlercolor.h"
#include <QColor>
#include <util/sll/qtutil.h>
#include "../widgets/colorpicker.h"

namespace LC
{
	ItemRepresentation HandleColor (const ItemContext& ctx)
	{
		const auto picker = new ColorPicker { ctx.Label_ };
		SetChangedSignal (ctx, picker, &ColorPicker::currentColorChanged);
		return
		{
			.Widget_ = picker,

			.DefaultValue_ = QColor { ctx.Default_ },
			.Getter_ = [picker] { return picker->GetCurrentColor (); },
			.Setter_ = [picker] (const QVariant& val) { picker->SetCurrentColor (val.value<QColor> ()); },
		};
	}
}
