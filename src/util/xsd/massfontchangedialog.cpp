/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "massfontchangedialog.h"
#include <util/sll/qtutil.h>

namespace LC
{
namespace Util
{
	MassFontChangeDialog::MassFontChangeDialog (const QFont& font,
			const QList<IWkFontsSettable::FontFamily>& families, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		Family2Box_ [IWkFontsSettable::FontFamily::StandardFont] = Ui_.StandardBox_;
		Family2Box_ [IWkFontsSettable::FontFamily::FixedFont] = Ui_.FixedBox_;
		Family2Box_ [IWkFontsSettable::FontFamily::SerifFont] = Ui_.SerifBox_;
		Family2Box_ [IWkFontsSettable::FontFamily::SansSerifFont] = Ui_.SansSerifBox_;
		Family2Box_ [IWkFontsSettable::FontFamily::CursiveFont] = Ui_.CursiveBox_;
		Family2Box_ [IWkFontsSettable::FontFamily::FantasyFont] = Ui_.FantasyBox_;

		for (const auto family : families)
			Family2Box_ [family]->setCheckState (Qt::Checked);

		Ui_.FontChooser_->SetFont (font);
	}

	QFont MassFontChangeDialog::GetFont () const
	{
		return Ui_.FontChooser_->GetFont ();
	}

	QList<IWkFontsSettable::FontFamily> MassFontChangeDialog::GetFamilies () const
	{
		QList<IWkFontsSettable::FontFamily> result;
		for (const auto& pair : Util::Stlize (Family2Box_))
			if (pair.second->checkState () == Qt::Checked)
				result << pair.first;
		return result;
	}
}
}
