/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemcategoryselector.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	ItemCategorySelector::ItemCategorySelector (QWidget *parent)
	: Util::CategorySelector { parent }
	{
		SetCaption (tr ("Items categories"));
		setWindowFlags (Qt::Widget);
		setMinimumHeight (0);
		SetButtonsMode (Util::CategorySelector::ButtonsMode::NoButtons);

		XmlSettingsManager::Instance ().RegisterObject ("ShowCategorySelector", this,
				[this] (bool visible)
				{
					CanBeVisible_ = visible;
					UpdateVisibility ();
				});
	}

	void ItemCategorySelector::SetPossibleSelections (QStringList selections, bool sort)
	{
		Util::CategorySelector::SetPossibleSelections (selections, sort);
		SelectAll ();
		UpdateVisibility ();
	}

	void ItemCategorySelector::UpdateVisibility ()
	{
		const auto& hasTags = !GetPossibleSelections ().isEmpty ();
		const auto visible = hasTags && CanBeVisible_;
		setVisible (visible);
	}
}
