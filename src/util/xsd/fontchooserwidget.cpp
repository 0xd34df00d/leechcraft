/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fontchooserwidget.h"

namespace LC
{
namespace Util
{
	FontChooserWidget::FontChooserWidget (QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		connect (Ui_.FontBox_,
				SIGNAL (currentFontChanged (QFont)),
				this,
				SIGNAL (fontChanged (QFont)));
	}

	QFont FontChooserWidget::GetFont () const
	{
		return Ui_.FontBox_->currentFont ();
	}

	void FontChooserWidget::SetFont (const QFont& font)
	{
		Ui_.FontBox_->setCurrentFont (font);
	}
}
}
