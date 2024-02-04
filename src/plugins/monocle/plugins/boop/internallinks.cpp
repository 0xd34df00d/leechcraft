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
			for (auto tag : Util::DomDescendants (root, tagName))
				if (const auto& link = tag.attribute (linkAttr);
					!link.isEmpty ())
				{
					const auto linkUrl = QUrl::fromEncoded (link.toLatin1 ());
					if (linkUrl.isRelative ())
						tag.setAttribute (linkAttr, baseUrl.resolved (linkUrl).toString ());
				}
		}

		QString NormalizeForId (QString id)
		{
			return id.replace ('/', '_').replace ('.', '_');
		}

		static const QString RootMarker = "lc_chapter_root"_qs;

		QString InternalizeLinkTarget (const QUrl& url)
		{
			const auto& fragment = url.hasFragment () ? url.fragment (QUrl::FullyEncoded) : RootMarker;
			const auto& path = url.path (QUrl::FullyEncoded);
			return NormalizeForId (path) + '_' + fragment;
		}
	}

	QString InternalizeLinkTarget (QString target)
	{
		auto url = QUrl::fromEncoded (target.toUtf8 ());
		if (!url.host ().isEmpty ())
			return target;
		return InternalizeLinkTarget (url);
	}

	namespace
	{
		void FixupIdAnchors (QDomElement elem, const QString& normalizedSubpath)
		{
			if (const auto& id = elem.attribute ("id"_qs);
				!id.isEmpty ())
				elem.setAttribute ("id"_qs, normalizedSubpath + '_' + id);

			for (const auto& child : Util::DomChildren (elem, {}))
				FixupIdAnchors (child.toElement (), normalizedSubpath);
		}

		void FixupHrefTargets (const QDomElement &root, const QUrl& baseUrl)
		{
			for (auto link : Util::DomDescendants (root, "a"_qs))
			{
				auto href = link.attribute ("href"_qs);
				if (href.isEmpty () || !QUrl::fromEncoded (href.toUtf8 ()).isRelative ())
					continue;

				const auto& resolvedHref = href.startsWith ('#') ?
						baseUrl.resolved (QUrl::fromEncoded (href.toUtf8 ())) :
						href;
				link.setAttribute ("href"_qs, '#' + InternalizeLinkTarget (resolvedHref));
			}
		}

		void MarkChapterElem (QDomElement root)
		{
			auto body = root.elementsByTagName ("body"_qs).at (0);
			auto firstElem = body.firstChildElement ();
			if (!firstElem.hasAttribute ("id"_qs))
				firstElem.setAttribute ("id"_qs, RootMarker);
			else
			{
				auto dummyChapterElem = root.ownerDocument ().createElement ("div"_qs);
				dummyChapterElem.setAttribute ("id"_qs, RootMarker);
				body.insertBefore (dummyChapterElem, firstElem);
			}
		}
	}

	void FixDocumentLinks (const QDomElement& root, const QString& subpath)
	{
		const QUrl chapterBaseUrl { subpath };
		ResolveLinks ("img"_qs, "src"_qs, root, chapterBaseUrl);
		ResolveLinks ("link"_qs, "href"_qs, root, chapterBaseUrl);
		ResolveLinks ("a"_qs, "href"_qs, root, chapterBaseUrl);

		MarkChapterElem (root);

		FixupHrefTargets (root, chapterBaseUrl);

		FixupIdAnchors (root, NormalizeForId (subpath));
	}
}
