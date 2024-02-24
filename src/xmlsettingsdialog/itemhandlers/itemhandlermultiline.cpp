/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlermultiline.h"
#include <QApplication>
#include <QTextEdit>
#include <util/sll/qtutil.h>
#include "../xmlsettingsdialog.h"
#include "defaultvaluegetters.h"

namespace LC
{
	ItemRepresentation HandleMultiline (const ItemContext& ctx)
	{
		const auto& item = ctx.Elem_;

		const auto edit = new QTextEdit;
		edit->setMinimumWidth (edit->fontMetrics ().horizontalAdvance ("thisismaybeadefaultsetting"_qs));
		SetChangedSignal (ctx, edit, &QTextEdit::textChanged);

		auto labelPos = LabelPosition::Default;
		if (item.attribute ("position"_qs) == "bottom"_ql)
			labelPos = LabelPosition::Wrap;

		return
		{
			.Widget_ = edit,
			.LabelPosition_ = labelPos,

			.DefaultValue_ = GetDefaultStringValue (ctx.Default_, ctx.XSD_.GetTrContext ()),
			.Getter_ = [edit] { return edit->toPlainText ().split ('\n', Qt::SkipEmptyParts); },
			.Setter_ = [edit] (const QVariant& value) { edit->setPlainText (value.toStringList ().join ('\n')); },
		};
	}
}
