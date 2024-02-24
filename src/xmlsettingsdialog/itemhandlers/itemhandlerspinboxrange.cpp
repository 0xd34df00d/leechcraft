/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerspinboxrange.h"
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "../widgets/rangewidget.h"

namespace LC
{
	namespace
	{
		QVariant GetDefaultRange (const QString& def)
		{
			const auto& parts = def.splitRef (':');
			if (parts.size () != 2)
			{
				qWarning () << "spinboxrange parse error, wrong default value";
				return {};
			}
			return QList<QVariant> { parts.at (0).toInt (), parts.at (1).toInt () };
		}
	}

	ItemRepresentation HandleSpinboxRange (const ItemContext& ctx)
	{
		const auto& item = ctx.Elem_;

		const auto range = new RangeWidget;
		range->SetBounds (item.attribute ("minimum"_qs).toInt (), item.attribute ("maximum"_qs).toInt ());

		SetChangedSignal (ctx, range, &RangeWidget::changed);

		return
		{
			.Widget_ = range,
			.DefaultValue_ = GetDefaultRange (ctx.Default_),
			.Getter_ = [range] { return range->GetRange (); },
			.Setter_ = [range] (const QVariant& value) { range->SetRange (value); },
		};
	}
}
