/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "manifest.h"
#include <QUrl>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include "util.h"

namespace LC::Monocle::Boop
{
	namespace
	{
		QString FindOpfFile (const QString& epubFile)
		{
			const auto& doc = GetXml (epubFile, "META-INF/container.xml"_qs);
			const auto& rootfiles = GetElem (doc.documentElement (), "rootfiles"_qs);
			const auto& rootfile = GetElem (rootfiles, "rootfile"_qs);
			return GetAttr (rootfile, "full-path"_qs);
		}
	}

	Manifest ParseManifest (const QString& epubFile)
	{
		const auto& opfFile = FindOpfFile (epubFile);
		const QUrl opfFileUrl { opfFile };

		const auto& doc = GetXml (epubFile, opfFile);

		Manifest manifest;
		for (const auto& item : Util::DomChildren (GetElem (doc.documentElement (), "manifest"_qs), "item"_qs))
		{
			const auto& id = GetAttr (item, "id"_qs);
			const auto& href = GetAttr (item, "href"_qs);
			const auto& resolvedPath = opfFileUrl.resolved (QUrl { href }).toString ();
			const auto& mime = GetAttr (item, "media-type"_qs);
			manifest.Id2Item_ [id] = PathItem { resolvedPath, mime };
		}

		const auto& spine = GetElem (doc.documentElement(), "spine"_qs);
		for (const auto& item : Util::DomChildren (spine, "itemref"_qs))
			manifest.Spine_ << GetAttr (item, "idref"_qs);

		return manifest;
	}
}
