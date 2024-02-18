/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerpushbutton.h"
#include <QGridLayout>
#include <QPushButton>
#include "../xmlsettingsdialog.h"

namespace LC
{
	ItemRepresentation HandlePushButton (const ItemContext& ctx)
	{
		auto& xsd = ctx.XSD_;

		const auto button = new QPushButton { ctx.Label_ };
		QObject::connect (button,
				&QPushButton::released,
				[prop = ctx.Prop_, &xsd] { emit xsd.pushButtonClicked (prop); });
		return
		{
			.Widget_ = button,
			.LabelPosition_ = LabelPosition::None,
		};
	}
}
