/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>
#include <QMetaType>
#include "ilink.h"

class QRectF;
class QDateTime;
class QPolygonF;

namespace LC
{
namespace Monocle
{
	/** @brief Defines an annotation type.
	 */
	enum class AnnotationType
	{
		/** @brief A simple text annotation.
		 *
		 * The corresponding interface is ITextAnnotation.
		 */
		Text,

		/** @brief A highlighted block in the text.
		 *
		 * The corresponding interface is IHighlightAnnotation.
		 */
		Highlight,

		/** @brief An annotation with a link.
		 *
		 * The corresponding interface is ILinkAnnotation.
		 */
		Link,

		/** @brief A caret pointing to some text inserted.
		 *
		 * The corresponding interface is ICaretAnnotation.
		 */
		Caret,

		/** @brief Another type of annotation.
		 */
		Other
	};

	/** @brief Base interface for annotations.
	 *
	 * This interface should be implemented by all annotation objects.
	 * In fact, exact annotation interfaces all derive from this one, so
	 * there is no need in deriving from this one explicitly.
	 *
	 * @sa ITextAnnotation, IHighlightAnnotation, ILinkAnnotation
	 */
	class IAnnotation
	{
	public:
		virtual ~IAnnotation () {}

		/** @brief Returns the author of the annotation.
		 *
		 * @return The name of the author of the annotation.
		 */
		virtual QString GetAuthor () const = 0;

		/** @brief Returns the date the annotation was created.
		 *
		 * If the date is unknown or not applicable, an invalid QDateTime
		 * object should be returned.
		 *
		 * @return The timestamp of the annotation.
		 */
		virtual QDateTime GetDate () const = 0;

		/** @brief Returns the bounding rectangle of the annotation.
		 *
		 * This method should return the bounding rectangle in page
		 * coordinates, where (0; 0) is the top left corner, and (1; 1)
		 * is the bottom right corner.
		 *
		 * @return The annotation rect in page coordinates.
		 */
		virtual QRectF GetBoundary () const = 0;

		/** @brief Returns the type of the annotation.
		 *
		 * @return The type of the annotation.
		 */
		virtual AnnotationType GetAnnotationType () const = 0;

		/** @brief Returns the text contained in the annotation.
		 *
		 * @return The text of the annotation (or an empty string if
		 * not applicable).
		 */
		virtual QString GetText () const = 0;
	};

	/** @brief The interface for ::AnnotationType::Text annotations.
	 */
	class ITextAnnotation : public IAnnotation
	{
	public:
		virtual ~ITextAnnotation () {}

		/** @brief Returns whether this is an inline annotation.
		 *
		 * Inline annotation should rather be displayed right on the
		 * document.
		 *
		 * @return Whether this is an inline annotation.
		 */
		virtual bool IsInline () const = 0;
	};

	/** @brief The interface for ::AnnotationType::Highlight annotations.
	 */
	class IHighlightAnnotation : public IAnnotation
	{
	public:
		virtual ~IHighlightAnnotation () {}

		/** @brief Returns the shape of the highlight.
		 *
		 * The shape of a single annotation is comprised of a list of
		 * polygons, each expected to be a closed shape. The polygons in
		 * the returned list can have both empty and non-empty pairwise
		 * intersections.
		 *
		 * The points in the polygons should be in page coordinates, where
		 * (0; 0) is the top left corner, and (1; 1) is the bottom right
		 * corner.
		 *
		 * The IAnnotation::GetBoundary() should return the bounding
		 * rectangle of the bounding rectangles of the polygons in the
		 * returned list.
		 *
		 * @return The shape of the highlight on the page in page coordinates.
		 */
		virtual QList<QPolygonF> GetPolygons () const = 0;
	};

	/** @brief The interface for ::AnnotationType::Link annotations.
	 *
	 * Please note that there shouldn't be link annotations that contain
	 * links equivalent to the ones returned from the
	 * IDocument::GetPageLinks() method. The corresponding links should
	 * be returned from the latter method.
	 *
	 * @sa ILink
	 * @sa IDocument::GetPageLinks()
	 */
	class ILinkAnnotation : public IAnnotation
	{
	public:
		virtual ~ILinkAnnotation () {}

		/** @brief Returns the link corresponding to this annotation.
		 *
		 * @return The link corresponding to this annotation.
		 */
		virtual ILink_ptr GetLink () const = 0;
	};

	/** @brief The interface for ::AnnotationType::Caret annotations.
	 */
	class ICaretAnnotation : public IAnnotation
	{
	public:
		virtual ~ICaretAnnotation () {}
	};

	typedef std::shared_ptr<IAnnotation> IAnnotation_ptr;
	typedef std::shared_ptr<ITextAnnotation> ITextAnnotation_ptr;
	typedef std::shared_ptr<IHighlightAnnotation> IHighlightAnnotation_ptr;
	typedef std::shared_ptr<ILinkAnnotation> ILinkAnnotation_ptr;
	typedef std::shared_ptr<ICaretAnnotation> ICaretAnnotation_ptr;
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IAnnotation,
		"org.LeechCraft.Monocle.IAnnotation/1.0")
Q_DECLARE_INTERFACE (LC::Monocle::ITextAnnotation,
		"org.LeechCraft.Monocle.ITextAnnotation/1.0")
Q_DECLARE_INTERFACE (LC::Monocle::IHighlightAnnotation,
		"org.LeechCraft.Monocle.IHighlightAnnotation/1.0")
Q_DECLARE_INTERFACE (LC::Monocle::ILinkAnnotation,
		"org.LeechCraft.Monocle.ILinkAnnotation/1.0")
Q_DECLARE_INTERFACE (LC::Monocle::ICaretAnnotation,
		"org.LeechCraft.Monocle.ICaretAnnotation/1.0")

Q_DECLARE_METATYPE (LC::Monocle::IAnnotation_ptr)
