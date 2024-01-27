/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "resourcedtextdocument.h"
#include <QtDebug>

namespace LC::Monocle
{
	ResourcedTextDocument::ResourcedTextDocument (const LazyImages_t& images)
	: Images_ { images }
	{
	}

	void ResourcedTextDocument::SetMaxImageSizes (const QHash<QUrl, QSize>& sizes)
	{
		MaxImageSizes_ = sizes;
	}

	QVariant ResourcedTextDocument::loadResource (int type, const QUrl& name)
	{
		if (type != QTextDocument::ImageResource)
			return QTextDocument::loadResource (type, name);

		const auto& image = Images_.value (name);
		if (!image.Load_)
			return QTextDocument::loadResource (type, name);

		const auto& maxSize = MaxImageSizes_.value (name);
		return image.Load_ (maxSize.isValid () ? maxSize : image.NativeSize_);
	}
}
