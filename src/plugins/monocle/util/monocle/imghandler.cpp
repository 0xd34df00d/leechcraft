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

	namespace
	{
		std::optional<QSize> GetImageSize (const LazyImage& image, const CustomStyler_f& styler, const StylingContext& ctx)
		{
			if (!image)
				return {};

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
	}

	void ImgHandler::HandleImg (const QDomElement& elem, const StylingContext& ctx)
	{
		QTextImageFormat imgFmt;
		imgFmt.setName (elem.attribute ("src"_qs));
		const auto& image = Images_.value (imgFmt.name ());
		if (!image)
			qWarning () << "unknown image" << imgFmt.name ();
		if (const auto& size = GetImageSize (image, Styler_, ctx))
		{
			imgFmt.setWidth (size->width ());
			imgFmt.setHeight (size->height ());
		}
		Cursor_.insertImage (imgFmt);
	}
}
