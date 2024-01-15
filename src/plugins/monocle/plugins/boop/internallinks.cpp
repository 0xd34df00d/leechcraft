/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "internallinks.h"
#include <QDomElement>
#include <QUrl>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>

namespace LC::Monocle::Boop
{
	namespace
	{
		void ResolveLinks (const QString& tagName, const QString& linkAttr, const QDomElement& root, const QUrl& baseUrl)
		{
			for (auto image : Util::DomDescendants (root, tagName))
				if (const auto& link = image.attribute (linkAttr);
					!link.isEmpty ())
				{
					const auto linkUrl = QUrl::fromEncoded (link.toLatin1 ());
					if (linkUrl.isRelative ())
						image.setAttribute (linkAttr, baseUrl.resolved (linkUrl).toString ());
				}
		}
	}

	QString InternalizeAnchor (QString anchor)
	{
		return anchor.replace ('#', '_').replace ('/', '_');
	}

	namespace
	{
		void FixupLinkHrefAnchors (const QDomElement &root)
		{
			for (auto link : Util::DomDescendants (root, "a"_qs))
			{
				auto href = link.attribute ("href"_qs);
				if (href.isEmpty () || !QUrl::fromEncoded (href.toUtf8 ()).isRelative ())
					continue;

				link.setAttribute ("href"_qs, InternalizeAnchor (std::move (href)).prepend ('#'));
			}
		}

		void FixupIdAnchors (QDomElement elem, const QString& subpath)
		{
			if (const auto& id = elem.attribute ("id"_qs);
					!id.isEmpty ())
				elem.setAttribute ("id"_qs, InternalizeAnchor (subpath + '#' + id));

			for (const auto& child : Util::DomChildren (elem, {}))
				FixupIdAnchors (child.toElement (), subpath);
		}
	}

	void FixLinks (const QDomElement& root, const QString& subpath)
	{
		const QUrl chapterBaseUrl { subpath };
		ResolveLinks ("img"_qs, "src"_qs, root, chapterBaseUrl);
		ResolveLinks ("link"_qs, "href"_qs, root, chapterBaseUrl);

		ResolveLinks ("a"_qs, "href"_qs, root, chapterBaseUrl);
		FixupLinkHrefAnchors (root);
		FixupIdAnchors (root, subpath);
	}
}
