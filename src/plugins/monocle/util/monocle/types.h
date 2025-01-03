/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QImage>
#include <QPair>
#include <QString>
#include <QTextCharFormat>
#include <QVector>

namespace LC::Monocle
{
	Q_DECL_IMPORT extern const QString TocSectionIdAttr;

	struct Span
	{
		int Start_;
		int End_;
		// Double-check Q_DECLARE_TYPEINFO when updating this type.
	};

	struct InternalLink
	{
		QString LinkTitle_;
		Span Link_;
		Span Target_;
		// Double-check Q_DECLARE_TYPEINFO when updating this type.
	};

	struct LazyImage
	{
		QSize NativeSize_;
		std::function<QImage (QSize)> Load_;

		explicit operator bool () const
		{
			return static_cast<bool> (Load_);
		}
	};
	using LazyImages_t = QHash<QUrl, LazyImage>;

	struct BlockFormat
	{
		std::optional<Qt::AlignmentFlag> Align_ {};

		/** Margins in px.
		 */
		std::optional<int> MarginLeft_ {};
		std::optional<int> MarginTop_ {};
		std::optional<int> MarginRight_ {};
		std::optional<int> MarginBottom_ {};

		/** First line indent in px.
		 */
		std::optional<int> Indent_ {};

		/** Heading level (as in h1-h6).
		 */
		std::optional<int> HeadingLevel_ {};

		std::optional<QBrush> Background_ {};
		std::optional<QBrush> Foreground_ {};

		BlockFormat& operator+= (const BlockFormat&);
	};

	struct CharFormat
	{
		std::optional<qreal> PointSize_ {};

		std::optional<QFont::Weight> IsBold_ {};
		std::optional<bool> IsItalic_ {};
		std::optional<bool> IsUnderline_ {};
		std::optional<bool> IsStrikeThrough_ {};

		std::optional<QTextCharFormat::VerticalAlignment> VerticalAlignment_ {};

		std::optional<QBrush> Background_ {};
		std::optional<QBrush> Foreground_ {};

		CharFormat& operator+= (const CharFormat&);

		bool IsEmpty () const;
	};

	struct ImageFormat
	{
		std::optional<qreal> Width_ {};
		std::optional<qreal> Height_ {};
	};

	struct StylingContextElement
	{
		QStringView Tag_;
		QList<QStringView> Classes_;
	};

	struct StylingContext
	{
		StylingContextElement Elem_;
		QVector<StylingContextElement> Parents_;

		const QTextCharFormat& CharFormat_;
	};

	struct Style
	{
		BlockFormat Block_;
		CharFormat Char_;
		ImageFormat Img_;
	};

	using CustomStyler_f = std::function<Style (const StylingContext&)>;
}

template<>
class QTypeInfo<LC::Monocle::Span> : public QTypeInfo<int> {};

template<>
class QTypeInfo<LC::Monocle::InternalLink> : public QTypeInfoMerger<QString, LC::Monocle::Span> {};
