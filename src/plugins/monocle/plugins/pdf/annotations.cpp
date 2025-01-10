/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "annotations.h"
#include <QPolygonF>
#include <QtDebug>
#include <poppler-annotation.h>
#include <poppler-version.h>
#include "links.h"

namespace LC::Monocle::PDF
{
	namespace
	{
		template<typename Target, typename Src>
		std::unique_ptr<Target> UniqueCast (std::unique_ptr<Src> src)
		{
			if (auto ptr = dynamic_cast<Target*> (src.get ()))
			{
				src.release ();
				return std::unique_ptr<Target> { ptr };
			}
			return {};
		}
	}

	IAnnotation_ptr MakeAnnotation (Document *doc, std::unique_ptr<Poppler::Annotation> ann)
	{
		switch (ann->subType ())
		{
		case Poppler::Annotation::SubType::AText:
			return std::make_shared<TextAnnotation> (UniqueCast<Poppler::TextAnnotation> (std::move (ann)));
		case Poppler::Annotation::SubType::AHighlight:
			return std::make_shared<HighlightAnnotation> (UniqueCast<Poppler::HighlightAnnotation> (std::move (ann)));
		case Poppler::Annotation::SubType::ALink:
			if (ann->contents ().isEmpty ())
				return {};
			else
				return std::make_shared<LinkAnnotation> (doc, UniqueCast<Poppler::LinkAnnotation> (std::move (ann)));
		case Poppler::Annotation::SubType::ACaret:
			return std::make_shared<CaretAnnotation> (UniqueCast<Poppler::CaretAnnotation> (std::move (ann)));
		default:
			break;
		}

		qWarning () << "unknown" << ann->subType ();
		return {};
	}

	AnnotationType TextAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Text;
	}

	bool TextAnnotation::IsInline () const
	{
		return Ann_->flags () & Poppler::TextAnnotation::InPlace;
	}

	AnnotationType HighlightAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Highlight;
	}

	QList<QPolygonF> HighlightAnnotation::GetPolygons () const
	{
		QList<QPolygonF> result;
		for (const auto& quad : Ann_->highlightQuads ())
			result.append (QPolygonF { QList<QPointF> { std::begin (quad.points), std::end (quad.points) } });
		return result;
	}

	LinkAnnotation::LinkAnnotation (Document *doc, std::unique_ptr<Poppler::LinkAnnotation> ann)
	: AnnotationBase { std::move (ann) }
	, Link_ { new Link { *doc, *Ann_->linkDestination () } }
	{
	}

	AnnotationType LinkAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Link;
	}

	ILink_ptr LinkAnnotation::GetLink () const
	{
		return Link_;
	}

	AnnotationType CaretAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Caret;
	}
}
