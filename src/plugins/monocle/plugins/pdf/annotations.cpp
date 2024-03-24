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

namespace LC
{
namespace Monocle
{
namespace PDF
{
	IAnnotation_ptr MakeAnnotation (Document *doc, Poppler::Annotation *ann)
	{
		switch (ann->subType ())
		{
		case Poppler::Annotation::SubType::AText:
			return std::make_shared<TextAnnotation> (dynamic_cast<Poppler::TextAnnotation*> (ann));
		case Poppler::Annotation::SubType::AHighlight:
			return std::make_shared<HighlightAnnotation> (dynamic_cast<Poppler::HighlightAnnotation*> (ann));
		case Poppler::Annotation::SubType::ALink:
			if (ann->contents ().isEmpty ())
				return {};
			else
				return std::make_shared<LinkAnnotation> (doc, dynamic_cast<Poppler::LinkAnnotation*> (ann));
		case Poppler::Annotation::SubType::ACaret:
			return std::make_shared<CaretAnnotation> (dynamic_cast<Poppler::CaretAnnotation*> (ann));
		default:
			break;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown"
				<< ann->subType ();

		return {};
	}

	TextAnnotation::TextAnnotation (Poppler::TextAnnotation *ann)
	: AnnotationBase { ann }
	, TextAnn_ { ann }
	{
	}

	AnnotationType TextAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Text;
	}

	bool TextAnnotation::IsInline () const
	{
		return TextAnn_->flags () & Poppler::TextAnnotation::InPlace;
	}

	HighlightAnnotation::HighlightAnnotation (Poppler::HighlightAnnotation *ann)
	: AnnotationBase { ann }
	, HighAnn_ { ann }
	{
	}

	AnnotationType HighlightAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Highlight;
	}

	QList<QPolygonF> HighlightAnnotation::GetPolygons () const
	{
		QList<QPolygonF> result;
		for (const auto& quad : HighAnn_->highlightQuads ())
			result.append ({ { std::begin (quad.points), std::end (quad.points) } });
		return result;
	}

	LinkAnnotation::LinkAnnotation (Document *doc, Poppler::LinkAnnotation *ann)
	: AnnotationBase { ann }
	, LinkAnn_ { ann }
	, Link_ { new Link { doc, LinkAnn_->linkDestination (), {} } }
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

	CaretAnnotation::CaretAnnotation (Poppler::CaretAnnotation *ann)
	: AnnotationBase { ann }
	{
	}

	AnnotationType CaretAnnotation::GetAnnotationType () const
	{
		return AnnotationType::Caret;
	}
}
}
}
