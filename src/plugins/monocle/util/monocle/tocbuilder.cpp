/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tocbuilder.h"
#include <QAbstractTextDocumentLayout>
#include <QDomElement>
#include <QTextBlock>
#include <QTextCursor>
#include <util/sll/qtutil.h>
#include <interfaces/monocle/idocument.h>

namespace LC::Monocle
{
	TocBuilder::TocBuilder (const QTextCursor& cursor, IDocument& monocleDoc)
	: Cursor_ { cursor }
	, MonocleDoc_ { monocleDoc }
	{
		CurrentSectionPath_.push (&Root_);
	}

	TOCEntryLevel_t TocBuilder::GetTOC () const
	{
		return Root_.ChildLevel_;
	}

	namespace
	{
		class TOCLink : public ILink
					  , public IPageLink
		{
			IDocument& Doc_;
			const QTextBlock Block_;
		public:
			TOCLink (IDocument& doc, const QTextBlock& block)
			: Doc_ { doc }
			, Block_ { block }
			{
			}

			LinkType GetLinkType () const override
			{
				return LinkType::PageLink;
			}

			QRectF GetArea () const override
			{
				return {};
			}

			void Execute () override
			{
				Doc_.navigateRequested ({}, { .Page_ = GetPageNumber (), .PagePosition_ = { NewX (), NewY () } });
			}

			QString GetDocumentFilename () const override
			{
				return {};
			}

			int GetPageNumber () const override
			{
				const auto layout = Block_.document ()->documentLayout ();
				const auto shiftY = layout->blockBoundingRect (Block_).toRect ().top ();
				const auto pageSize = Block_.document ()->pageSize ().height ();
				return shiftY / pageSize;
			}

			double NewX () const override
			{
				return 0;
			}

			double NewY () const override
			{
				return 0;
			}

			double NewZoom () const override
			{
				return 0;
			}
		};
	}

	Util::DefaultScopeGuard TocBuilder::HandleElem (const QDomElement& elem)
	{
		const auto& sectionTitle = elem.attribute ("section-title"_qs);
		if (sectionTitle.isEmpty ())
			return {};

		auto& curLevel = CurrentSectionPath_.top ()->ChildLevel_;
		auto link = std::make_shared<TOCLink> (MonocleDoc_, Cursor_.block ());
		curLevel.append ({ .Link_ = std::move (link), .Name_ = sectionTitle, .ChildLevel_ = {} });
		CurrentSectionPath_.push (&curLevel.back ());

		return Util::MakeScopeGuard ([this] { CurrentSectionPath_.pop (); });
	}
}
