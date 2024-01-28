/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "resourcedtextdocument.h"
#include <QtDebug>
#include <Magick++.h>
#include <util/sll/udls.h>

namespace LC::Monocle
{
	ResourcedTextDocument::ResourcedTextDocument (const LazyImages_t& images)
	: Images_ { images }
	, ImagesCache_ { 10_mib }
	{
	}

	void ResourcedTextDocument::SetMaxImageSizes (const QHash<QUrl, QSize>& sizes)
	{
		MaxImageSizes_ = sizes;
		ImagesCache_.clear ();
	}

	namespace
	{
		Magick::Geometry ToMagick (QSize size)
		{
			return { static_cast<size_t> (size.width ()), static_cast<size_t> (size.height ()) };
		}

		std::string ToMagick (QImage::Format fmt)
		{
			switch (fmt)
			{
			case QImage::Format_RGB32:
				return "RGBA";
			case QImage::Format_ARGB32:
				return "RGBA";
			default:
				qWarning () << "unsupported fmt" << fmt;
				throw std::runtime_error { "unsupported ToMagick fmt" };
			}
		}

		QImage Downscale (QImage&& image, QSize size)
		{
			if (size == image.size ())
				return image;

			const auto noSharpenFactor = 2;
			if (size.width () * noSharpenFactor > image.width () || size.height () * noSharpenFactor > image.height ())
				return image.scaled (size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			Magick::Image magick
			{
				static_cast<size_t> (image.width ()),
				static_cast<size_t> (image.height ()),
				ToMagick (image.format ()),
				Magick::CharPixel,
				image.constBits ()
			};
			magick.filterType (MagickCore::Lanczos2SharpFilter);
			magick.resize (ToMagick (size));
			magick.sharpen (2, 2);

			QImage result { size, QImage::Format_ARGB32 };
			magick.write (0, 0, size.width (), size.height (), ToMagick (result.format ()), Magick::CharPixel, result.bits ());
			return result;
		}
	}

	QVariant ResourcedTextDocument::loadResource (int type, const QUrl& name)
	{
		if (type != QTextDocument::ImageResource)
			return QTextDocument::loadResource (type, name);

		if (const auto image = ImagesCache_ [name])
			return *image;

		const auto& image = Images_.value (name);
		if (!image)
			return QTextDocument::loadResource (type, name);

		const auto& maxSize = MaxImageSizes_.value (name);
		const auto& loaded = Downscale (image.Load_ (image.NativeSize_), maxSize.isValid () ? maxSize : image.NativeSize_);
		ImagesCache_.insert (name, new QImage { loaded }, loaded.sizeInBytes ());
		return loaded;
	}
}
