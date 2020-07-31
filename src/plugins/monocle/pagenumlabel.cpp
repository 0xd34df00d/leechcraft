/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagenumlabel.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Monocle
{
	PageNumLabel::PageNumLabel (QWidget *parent)
	: QSpinBox { parent }
	{
		setSpecialValueText (" ");

		XmlSettingsManager::Instance ().RegisterObject ("InvertedPageNumLabel",
				this, [this] (const auto& invertedVar) { Inverted_ = invertedVar.toBool (); });
	}

	void PageNumLabel::SetTotalPageCount (int count)
	{
		blockSignals (true);
		setSpecialValueText ({});
		setSuffix (" / " + QString::number (count));
		setRange (1, count);
		blockSignals (false);
	}

	void PageNumLabel::SetCurrentPage (int page)
	{
		blockSignals (true);
		setValue (page + 1);
		blockSignals (false);
	}

	void PageNumLabel::stepBy (int steps)
	{
		if (Inverted_)
			steps = -steps;
		QAbstractSpinBox::stepBy (steps);
	}
}
}
