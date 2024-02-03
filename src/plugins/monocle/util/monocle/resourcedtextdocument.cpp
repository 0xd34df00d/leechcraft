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
#include "textdocumentformatconfig.h"

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

		bool IsSupportedFormat (QImage::Format fmt)
		{
			switch (fmt)
			{
			case QImage::Format_RGB32:
			case QImage::Format_ARGB32:
				return true;
			default:
				return false;
			}
		}

		int GetLightness (QRgb rgb)
		{
			return QColor { rgb }.lightness ();
		}

		/** Returns the fraction of the pixels that are brighter than bgColor.
		 */
		double GetBrightFraction (const QImage& image, const QColor& bgColor)
		{
			const auto bgLightness = GetLightness (bgColor.rgb ());

			const auto totalPixels = image.sizeInBytes () / 4;
			int brightCount = 0;

			const auto& rgbs = reinterpret_cast<const QRgb*> (image.constBits ());
			for (int i = 0; i < totalPixels; ++i)
				if (GetLightness (rgbs [i]) > bgLightness)
					++brightCount;
			return static_cast<double> (brightCount) / totalPixels;
		}

		QRgb ComputeSubtrahend (QColor bgColor)
		{
			int bgR = 0;
			int bgG = 0;
			int bgB = 0;
			bgColor.getRgb (&bgR, &bgG, &bgB);
			return qRgba (0xff - bgR, 0xff - bgG, 0xff - bgB, 0);
		}

		using Components = std::array<unsigned char, 4>;

		QImage InvertColors (QImage&& image)
		{
			image.invertPixels ();
			return image;
		}

		QImage AddBackground (QImage&& image, QColor bg)
		{
			if (bg == Qt::black)
				return image;

			bg.setAlpha (0);
			const auto addend = std::bit_cast<Components> (bg.rgba ());

			auto bytes = image.bits ();
			for (int i = 0, totalBytes = image.sizeInBytes (); i < totalBytes; i += 4)
				for (int j = 0; j < 4; ++j)
					bytes [i + j] += std::min (addend [j], static_cast<uchar> (0xff - bytes [i + j]));
			return image;
		}

		QImage SubtractBackground (QImage&& image, QColor bg)
		{
			const auto subtrahend = std::bit_cast<Components> (ComputeSubtrahend (bg));
			auto bytes = image.bits ();
			for (int i = 0, totalBytes = image.sizeInBytes (); i < totalBytes; i += 4)
				for (int j = 0; j < 4; ++j)
					bytes [i + j] -= std::min (subtrahend [j], bytes [i + j]);
			return image;
		}

		QImage AdjustColors (QImage&& image)
		{
			if (!IsSupportedFormat (image.format ()))
			{
				qWarning () << "unsupported image format" << image.format ();
				return image;
			}

			const auto& palette = TextDocumentFormatConfig::Instance ().GetPalette ();
			if (palette.Background_ == Qt::white)
				return image;

			if (GetBrightFraction (image, palette.Background_) < 0.1) // TODO make configurable
				return image;

			return palette.Background_.lightness () < palette.Foreground_.lightness () ?
					AddBackground (InvertColors (std::move (image)), palette.Background_) :
					SubtractBackground (std::move (image), palette.Background_);
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
			magick.filterType (MagickCore::LanczosRadiusFilter);
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

		auto loaded = image.Load_ (image.NativeSize_);
		auto colorAdjusted = AdjustColors (std::move (loaded));
		const auto& processed = Downscale (std::move (colorAdjusted), maxSize.isValid () ? maxSize : image.NativeSize_);
		ImagesCache_.insert (name, new QImage { processed }, processed.sizeInBytes ());

		return processed;
	}
}
