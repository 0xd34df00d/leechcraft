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
#include <util/monocle/types.h>
#include "internallinks.h"
#include "manifest.h"
#include "util.h"

namespace LC::Monocle::Boop
{
	namespace
	{
		void BuildMap (const QUrl& ncxSubpathUrl, TOCEntryID& root, const QDomElement& navMap)
		{
			for (const auto& navPoint : Util::DomChildren (navMap, "navPoint"_qs))
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
				TOCEntryID entry { .Navigation_ = InternalizeLinkTarget (target).toLatin1 (), .Name_ = label };
				BuildMap (ncxSubpathUrl, entry, navPoint);
				root.ChildLevel_ << entry;
			}
		}

		TOCEntryID LoadNcxTocMap (const QString& epubFile, const QString& ncxSubpath)
		{
			const auto& ncx = GetXml (epubFile, ncxSubpath);
			TOCEntryID toc;
			BuildMap (QUrl { ncxSubpath }, toc, ncx.documentElement ().firstChildElement ("navMap"_qs));
			return toc;
		}
	}

	TOCEntryID LoadTocMap (const QString& epubFile, const Manifest& manifest)
	{
		static constexpr auto NcxMime = "application/x-dtbncx+xml"_ql;

		for (const auto& item : manifest.Id2Item_)
			if (item.Mime_ == NcxMime)
				return LoadNcxTocMap (epubFile, item.Path_);

		return {};
	}

	namespace
	{
		QSet<QString> GetEntriesIds (const TOCEntryID& root)
		{
			QSet<QString> result;

			auto entries = root.ChildLevel_;
			while (!entries.isEmpty ())
			{
				const auto& entry = entries.takeLast ();
				result << entry.Navigation_;
				entries += entry.ChildLevel_;
			}

			return result;
		}

		void MarkTocTargets (QDomElement elem, const QSet<QString>& ids)
		{
			const auto& id = elem.attribute ("id"_qs);
			if (!id.isEmpty () && ids.contains (id))
				elem.setAttribute (TocSectionIdAttr, id);

			for (const auto& child : Util::DomChildren (elem, {}))
				MarkTocTargets (child, ids);
		}
	}

	void MarkTocTargets (const QDomElement& elem, const TOCEntryID& toc)
	{
		MarkTocTargets (elem, GetEntriesIds (toc));
	}
}
