/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QHash>
#include <interfaces/iwkfontssettable.h>
#include "ui_massfontchangedialog.h"

namespace LC::Util
{
	class MassFontChangeDialog : public QDialog
	{
		Ui::MassFontChangeDialog Ui_;

		QHash<IWkFontsSettable::FontFamily, QCheckBox*> Family2Box_;
	public:
		MassFontChangeDialog (const QFont&,
				const QList<IWkFontsSettable::FontFamily>&,
				QWidget* = nullptr);

		QFont GetFont () const;
		QList<IWkFontsSettable::FontFamily> GetFamilies () const;
	};
}
