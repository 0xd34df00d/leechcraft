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
	: CategorySelector { parent }
	{
		SetModel (Categories_);
		SetCaption (tr ("Items categories"));

		XmlSettingsManager::Instance ().RegisterObject ("ShowCategorySelector", this,
				[this] (bool visible)
				{
					CanBeVisible_ = visible;
					UpdateVisibility ();
				});
	}

	void ItemCategorySelector::SetCategories (const QStringList& categories)
	{
		Categories_.setStringList (categories);
		SelectAll ();
		UpdateVisibility ();
	}

	void ItemCategorySelector::UpdateVisibility ()
	{
		const auto hasCategories = Categories_.rowCount () > 0;
		setVisible (hasCategories && CanBeVisible_);
	}
}
