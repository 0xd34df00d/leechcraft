/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/iquarkcomponentprovider.h>
#include <util/qml/unhidelistviewbase.h>

namespace LC::SB2
{
	class ViewManager;
	class QuarkManager;

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;

	class QuarkUnhideListView : public Util::UnhideListViewBase
	{
		Q_OBJECT

		ViewManager *ViewManager_;

		struct ComponentInfo
		{
			QuarkComponent_ptr Comp_;
			QuarkManager_ptr Manager_;
		};
		QHash<QString, ComponentInfo> ID2Component_;
	public:
		QuarkUnhideListView (const QuarkComponents_t&, ViewManager*, QWidget*  = nullptr);
	private slots:
		void unhide (const QString&);
	};
}
