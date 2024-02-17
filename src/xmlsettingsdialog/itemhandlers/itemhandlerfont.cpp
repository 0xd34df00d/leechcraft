/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemhandlerfont.h"
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <util/sll/qtutil.h>
#include "../widgets/fontpicker.h"

namespace LC
{
	namespace
	{
		QVariant GetDefaultFont (const QDomElement& elem)
		{
			if (!elem.hasAttribute ("default"_qs))
				return QGuiApplication::font ();

			const auto& defStr = elem.attribute ("default"_qs);
			if (QFont font;
				font.fromString (defStr))
				return font;

			return QFont { defStr };
		}
	}

	ItemRepresentation HandleFont (const ItemContext& ctx)
	{
		const auto picker = new FontPicker { ctx.Label_ };
		SetChangedSignal (ctx, picker, &FontPicker::currentFontChanged);
		return
		{
			.Widget_ = picker,
			.Label_ = ctx.Label_,

			.DefaultValue_ = GetDefaultFont (ctx.Elem_),

			.Getter_ = [picker] { return picker->GetCurrentFont (); },
			.Setter_ = [picker] (const QVariant& value) { picker->SetCurrentFont (value.value<QFont> ()); },
		};
	}
}
