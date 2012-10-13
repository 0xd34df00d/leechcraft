/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <QMetaType>

namespace LeechCraft
{
namespace Blogique
{
	enum class SideWidgetType
	{
		PostOptionsSideWidget,
		CustomSideWidget
	};

	/** @brief Interface representing a side widget for blogique tab's dockwidget'.
		*
		**/
	class IBlogiqueSideWidget
	{
	public:
		virtual ~IBlogiqueSideWidget () {};

		/** @brief Returns human readable name of widget.
		*
		* @return A human readable widget name.
		**/
		virtual QString GetName () const = 0;

		/** @brief Returns a type of blogique side widget.
		*
		* If type of widget is SideWidgetType::PostOptionsSideWidget widget should
		* implement IPostOptionsWidget interface and ICustomSideWidget interface
		* if type of widget is SideWidgetType::CustomSideWidget.
		*
		* @return SideWidgetType of blogique side widget.
		*
		* @sa IPostOptionsWidget, ICustomSideWidget.
		**/
		virtual SideWidgetType GetWidgetType () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IBlogiqueSideWidget,
		"org.Deviant.LeechCraft.Blogique.IBlogiqueSideWidget/1.0");
