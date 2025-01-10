/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "qt5compat.h"
#include <poppler-annotation.h>
#include <interfaces/monocle/iannotation.h>

namespace Poppler
{
	class Annotation;
	class TextAnnotation;
}

namespace LC::Monocle::PDF
{
	class Document;

	IAnnotation_ptr MakeAnnotation (Document*, std::unique_ptr<Poppler::Annotation>);

	template<typename T, typename PT>
	class AnnotationBase : public T
	{
	protected:
		const std::unique_ptr<PT> Ann_;
	public:
		AnnotationBase (std::unique_ptr<PT> ann)
		: Ann_ { std::move (ann) }
		{
		}

		~AnnotationBase () override = default;

		QString GetAuthor () const override
		{
			return Ann_->author ();
		}

		QDateTime GetDate () const override
		{
			return Ann_->creationDate ();
		}

		PageRelativeRectBase GetBoundary () const override
		{
			return PageRelativeRectBase { Ann_->boundary () };
		}

		QString GetText () const override
		{
			return Ann_->contents ();
		}
	};

	class TextAnnotation : public AnnotationBase<ITextAnnotation, Poppler::TextAnnotation>
	{
	public:
		using AnnotationBase::AnnotationBase;

		AnnotationType GetAnnotationType () const override;
		bool IsInline () const override;
	};

	class HighlightAnnotation : public AnnotationBase<IHighlightAnnotation, Poppler::HighlightAnnotation>
	{
	public:
		using AnnotationBase::AnnotationBase;

		AnnotationType GetAnnotationType () const override;
		QList<QPolygonF> GetPolygons () const override;
	};

	class LinkAnnotation : public AnnotationBase<ILinkAnnotation, Poppler::LinkAnnotation>
	{
		ILink_ptr Link_;
	public:
		LinkAnnotation (Document*, std::unique_ptr<Poppler::LinkAnnotation>);

		AnnotationType GetAnnotationType () const override;
		ILink_ptr GetLink () const override;
	};

	class CaretAnnotation : public AnnotationBase<ICaretAnnotation, Poppler::CaretAnnotation>
	{
	public:
		using AnnotationBase::AnnotationBase;

		AnnotationType GetAnnotationType () const override;
	};
}
