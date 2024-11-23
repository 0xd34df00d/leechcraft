/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMetaType>
#include <QUrl>
#include "coordsbase.h"

namespace LC::Monocle
{
	/** @brief Describes various link types known to Monocle.
	 *
	 * All links implement the ILink interface.
	 *
	 * @sa ILink
	 */
	enum class LinkType
	{
		/** @brief A link to a page.
		 *
		 * The link may refer both the document it belongs to as well as
		 * some other document.
		 *
		 * The ILink::GetLinkAction() for `PageLink`s is supposed to return a
		 * `NavigationAction`.
		 *
		 * @sa NavigationAction
		 */
		PageLink,

		/** @brief A link to an URL.
		 */
		URL,

		/** @brief Some standard command like printing.
		 */
		Command,

		/** @brief Other link type.
		 */
		OtherLink
	};

	/** @brief A link action that represents navigating inside the document.
	 */
	struct NavigationAction
	{
		/** The (zero-based) index of the page this link refers to.
		 */
		int PageNumber_;

		/** The target rect of this link in the viewport.
		 */
		std::optional<PageRelativeRectBase> TargetArea_ {};

		/** The new zoom value for the page.
		 */
		std::optional<double> Zoom_ {};

		bool operator== (const NavigationAction&) const = default;
		bool operator< (const NavigationAction& other) const
		{
			const auto toTuple = [] (const NavigationAction& act)
			{
				qreal x {};
				qreal y {};
				qreal w {};
				qreal h {};
				act.TargetArea_.value_or (PageRelativeRectBase {}).ToRectF ().getRect (&x, &y, &w, &h);
				return std::tie (act.PageNumber_, x, y, w, h);  // don't care about the zoom
			};

			return toTuple (*this) < toTuple (other);
		}
	};

	/** @brief A link action that represents navigating to a different document.
	 */
	struct ExternalNavigationAction
	{
		/** The name of the document to open.
		 */
		QString TargetDocument_ {};

		/** The navigation action within the new document.
		 */
		NavigationAction DocumentNavigation_;
	};

	struct UrlAction
	{
		QUrl Url_;
	};

	using CustomAction = std::function<void ()>;

	struct NoAction {};

	using LinkAction = std::variant<NoAction, NavigationAction, ExternalNavigationAction, UrlAction, CustomAction>;

	/** @brief Base interface for links.
	 *
	 * Links should implement this interface.
	 */
	class ILink
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~ILink () = default;

		/** @brief Returns the link type.
		 *
		 * @return The type of this link.
		 */
		virtual LinkType GetLinkType () const = 0;

		/** @brief Returns the area on the page of this link.
		 *
		 * If the link doesn't belong to a page (i. e. is a TOC link) the
		 * return value isn't used and may be arbitrary.
		 *
		 * @return The area of this link on its page.
		 */
		virtual PageRelativeRectBase GetArea () const = 0;

		/** @brief Returns the link action.
		 */
		virtual LinkAction GetLinkAction () const = 0;

		/** @brief Returns the tooltip for the link.
		 */
		 virtual QString GetToolTip () const
		 {
			 return {};
		 }
	};
	typedef std::shared_ptr<ILink> ILink_ptr;
}

Q_DECLARE_INTERFACE (LC::Monocle::ILink, "org.LeechCraft.Monocle.ILink/1.0")
