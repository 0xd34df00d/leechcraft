/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imghandler.h"
#include <QDomElement>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextImageFormat>
#include <QUrl>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Monocle
{
	ImgHandler::ImgHandler (QTextCursor& cursor, const CustomStyler_f& styler, const LazyImages_t& images)
	: Cursor_ { cursor }
	, Styler_ { styler }
	, Images_ { images }
	{
	}

	QHash<QUrl, QSize> ImgHandler::GetMaxSizes () const
	{
		return Image2MaxSize_;
	}

	namespace
	{
		QSize GetImageSize (const LazyImage& image, const CustomStyler_f& styler, const StylingContext& ctx)
		{
			if (!styler)
				return image.NativeSize_;

			const auto& imgFmt = styler (ctx).Img_;

			if (imgFmt.Width_ && imgFmt.Height_)
				return image.NativeSize_.scaled ({ static_cast<int> (*imgFmt.Width_), static_cast<int> (*imgFmt.Height_) }, Qt::KeepAspectRatio);
			if (imgFmt.Width_)
				return image.NativeSize_ * (*imgFmt.Width_ / image.NativeSize_.width ());
			if (imgFmt.Height_)
				return image.NativeSize_ * (*imgFmt.Height_ / image.NativeSize_.height ());

			return image.NativeSize_;
		}

		QMarginsF ToMargins (const auto& fmt)
		{
			return { fmt.leftMargin (), fmt.topMargin (), fmt.rightMargin (), fmt.bottomMargin () };
		}

		QMarginsF GetFramesMargins (const QTextFrame *frame)
		{
			if (!frame)
				return {};
			return ToMargins (frame->frameFormat ()) + GetFramesMargins (frame->parentFrame ());
		}

		QSize BoundedToBlockArea (QSize size, const QTextCursor& cursor)
		{
			const auto margins = GetFramesMargins (cursor.currentFrame ()) + ToMargins (cursor.blockFormat ());
			const auto& areaSize = cursor.document ()->pageSize ().shrunkBy (margins);
			if (size.width () <= areaSize.width () && size.height () <= areaSize.height ())
				return size;
			return size.scaled (areaSize.toSize (), Qt::KeepAspectRatio);
		}
	}

	void ImgHandler::HandleImg (const QDomElement& elem, const StylingContext& ctx)
	{
		const auto& name = elem.attribute ("src"_qs);

		QTextImageFormat imgFmt;
		imgFmt.setName (name);
		if (const auto& image = Images_.value (name))
		{
			const auto& size = BoundedToBlockArea (GetImageSize (image, Styler_, ctx), Cursor_);

			imgFmt.setWidth (size.width ());
			imgFmt.setHeight (size.height ());

			auto& maxSize = Image2MaxSize_ [QUrl::fromEncoded (name.toUtf8 ())];
			maxSize = maxSize.expandedTo (size);
		}
		else
			qWarning () << "unknown image" << name;

		Cursor_.insertImage (imgFmt);
	}
}
