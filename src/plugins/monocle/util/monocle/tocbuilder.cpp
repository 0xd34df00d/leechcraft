/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tocbuilder.h"
#include <QDomElement>
#include <QTextCursor>
#include <QtDebug>
#include <interfaces/monocle/idocument.h>
#include "pagelink.h"

namespace LC::Monocle
{
	namespace
	{
		void TransformStructure (const TOCEntryID& toc,
				TOCEntryT<Span>& spanLevel,
				QHash<QByteArray, TOCEntryT<Span>*>& id2entry)
		{
			spanLevel.Name_ = toc.Name_;
			id2entry [toc.Navigation_] = &spanLevel;

			const auto childCount = toc.ChildLevel_.size ();
			spanLevel.ChildLevel_.resize (childCount);
			for (int i = 0; i < childCount; ++i)
				TransformStructure (toc.ChildLevel_ [i], spanLevel.ChildLevel_ [i], id2entry);
		}
	}

	TocBuilder::TocBuilder (const TOCEntryID& tocStructure, const QTextCursor& cursor)
	: Cursor_ { cursor }
	{
		TransformStructure (tocStructure, Root_, Id2Entry_);
	}

	TOCEntryLevelT<Span> TocBuilder::GetTOC () const
	{
		return Root_.ChildLevel_;
	}

	void TocBuilder::HandleElem (const QDomElement& elem)
	{
		const auto& sectionId = elem.attribute (TocSectionIdAttr);
		if (sectionId.isEmpty ())
			return;

		const auto entry = Id2Entry_.value (sectionId.toLatin1 ());
		if (!entry)
		{
			qWarning () << "unknown id"
					<< sectionId;
			return;
		}

		const auto curPosition = Cursor_.position ();
		entry->Navigation_ = { curPosition, curPosition };
	}
}
