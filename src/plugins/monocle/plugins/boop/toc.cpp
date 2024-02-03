/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toc.h"
#include <QDomElement>
#include <QtDebug>
#include <QUrl>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include "internallinks.h"
#include "manifest.h"
#include "util.h"

namespace LC::Monocle::Boop
{
	Toc_t LoadNcxTocMap (const QString& epubFile, const QString& ncxSubpath)
	{
		const auto& ncx = GetXml (epubFile, ncxSubpath);
		QVector<QDomElement> navMapQueue;
		navMapQueue << ncx.documentElement ().firstChildElement ("navMap"_qs);
		const QUrl ncxSubpathUrl { ncxSubpath };

		Toc_t toc;
		while (!navMapQueue.isEmpty ())
		{
			const auto& elem = navMapQueue.takeLast ();

			for (const auto& navPoint : Util::DomChildren (elem, "navPoint"_qs))
			{
				const auto& label = navPoint
						.firstChildElement ("navLabel"_qs)
						.firstChildElement ("text"_qs)
						.text ();
				const auto& src = navPoint
						.firstChildElement ("content"_qs)
						.attribute ("src"_qs);
				if (label.isEmpty () || src.isEmpty ())
				{
					qWarning () << "invalid nav point";
					continue;
				}
				const auto& target = ncxSubpathUrl.resolved (src).toString ();
				toc [InternalizeLinkTarget (target)] = label;

				navMapQueue << navPoint;
			}
		}
		return toc;
	}

	Toc_t LoadTocMap (const QString& epubFile, const Manifest& manifest)
	{
		static constexpr auto NcxMime = "application/x-dtbncx+xml"_ql;

		for (const auto& item : manifest.Id2Item_)
			if (item.Mime_ == NcxMime)
				return LoadNcxTocMap (epubFile, item.Path_);

		return {};
	}

	void MarkTocTargets (QDomElement elem, const Toc_t& toc)
	{
		const auto& id = elem.attribute ("id"_qs);
		if (!id.isEmpty ())
		{
			const auto& label = toc.value (id);
			if (!label.isEmpty ())
				elem.setAttribute ("section-title"_qs, label);
		}

		for (const auto& child : Util::DomChildren (elem, {}))
			MarkTocTargets (child, toc);
	}
}
