/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>
#include <QVariant>

namespace LC
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
		* implement IPostOptionsWidget interface.
		*
		* @return SideWidgetType of blogique side widget.
		*
		* @sa IPostOptionsWidget, ICustomSideWidget.
		**/
		virtual SideWidgetType GetWidgetType () const = 0;

		/** @brief Returns a map with post options.
		 *
		 * If type of widget is not a SideWidgetType::PostOptionsSideWidget widget
		 * should return empty map.
		 *
		 * @return QVariantMap with post options.
		 *
		 * @sa IPostOptionsWidget, ICustomSideWidget.
		 **/
		virtual QVariantMap GetPostOptions () const = 0;

		/** @brief Fill widget with post options.
		 *
		 * If type of widget is not a SideWidgetType::PostOptionsSideWidget widget
		 * shouldn't do anything.
		 *
		 * @sa IPostOptionsWidget, ICustomSideWidget.
		 **/
		virtual void SetPostOptions (const QVariantMap& map) = 0;

		/** @brief Returns a map with custom options.
		 *
		 * If type of widget is a SideWidgetType::PostOptionsSideWidget widget
		 * should return empty map.
		 *
		 * @return QVariantMap with custom options.
		 *
		 * @sa IPostOptionsWidget, ICustomSideWidget.
		 **/
		virtual QVariantMap GetCustomData () const = 0;

		/** @brief Fill widget with custom data.
		 *
		 * If type of widget is a SideWidgetType::PostOptionsSideWidget widget
		 * shouldn't do anything.
		 *
		 * @sa IPostOptionsWidget, ICustomSideWidget.
		 **/
		virtual void SetCustomData (const QVariantMap& map) = 0;

		/** @brief Set account object.
		 *
		 * @sa IAccount.
		 **/
		virtual void SetAccount (QObject *accountObj) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blogique::IBlogiqueSideWidget,
		"org.Deviant.LeechCraft.Blogique.IBlogiqueSideWidget/1.0")
