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

	void MarkTocTargets (const TOCEntryID& level, const QHash<QString, QDomElement>& id2elem)
	{
		if (!level.Navigation_.isEmpty ())
		{
			auto elem = id2elem [level.Navigation_];
			if (elem.isNull ())
				qWarning () << "unknown TOC target" << level.Navigation_;
			else
				elem.setAttribute (TocSectionIdAttr, QString::fromUtf8 (level.Navigation_));
		}

		for (const auto& item : level.ChildLevel_)
			MarkTocTargets (item, id2elem);
	}
}
