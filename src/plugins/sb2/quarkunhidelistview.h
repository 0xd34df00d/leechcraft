/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/iquarkcomponentprovider.h>
#include "unhidelistviewbase.h"

namespace LeechCraft
{
namespace SB2
{
	class ViewManager;
	class QuarkManager;

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;

	class QuarkUnhideListView : public UnhideListViewBase
	{
		Q_OBJECT

		ViewManager *ViewManager_;

		struct ComponentInfo
		{
			QuarkComponent Comp_;
			QuarkManager_ptr Manager_;
		};
		QHash<QString, ComponentInfo> ID2Component_;
	public:
		QuarkUnhideListView (const QList<QuarkComponent>&, ViewManager*, ICoreProxy_ptr, QWidget*  = 0);
	private slots:
		void unhide (const QString&);
	};
}
}
