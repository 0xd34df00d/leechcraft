/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerlineedit.h"
#include <QLineEdit>
#include <QApplication>
#include <util/sll/qtutil.h>
#include "../xmlsettingsdialog.h"
#include "defaultvaluegetters.h"

namespace LC
{
	ItemRepresentation HandleLineEdit (const ItemContext& ctx)
	{
		const auto& item = ctx.Elem_;

		const auto edit = new QLineEdit;
		edit->setMinimumWidth (QApplication::fontMetrics ().horizontalAdvance ("thisismaybeadefaultsetting"_qs));
		if (item.hasAttribute ("password"_qs))
			edit->setEchoMode (QLineEdit::Password);
		if (item.hasAttribute ("inputMask"_qs))
			edit->setInputMask (item.attribute ("inputMask"_qs));

		SetChangedSignal (ctx, edit, &QLineEdit::textChanged);

		return
		{
			.Widget_ = edit,
			.DefaultValue_ = GetDefaultStringValue (ctx.Elem_, ctx.XSD_.GetTrContext ()),
			.Getter_ = [edit] { return edit->text (); },
			.Setter_ = [edit] (const QVariant& value) { edit->setText (value.toString ()); },
		};
	}
}
