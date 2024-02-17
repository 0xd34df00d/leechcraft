/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlercustomwidget.h"
#include <QVBoxLayout>
#include <QWidget>

namespace LC
{
	ItemRepresentation HandleCustomWidget (const ItemContext&)
	{
		const auto widget = new QWidget {};
		const auto layout = new QVBoxLayout {};
		layout->setContentsMargins (0, 0, 0, 0);
		widget->setLayout (layout);
		widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		return
		{
			.Widget_ = widget,
			.LabelPosition_ = LabelPosition::None,
			.Getter_ = [] { return QVariant {}; },
			.Setter_ = [] (const QVariant&) {},
		};
	}
}
