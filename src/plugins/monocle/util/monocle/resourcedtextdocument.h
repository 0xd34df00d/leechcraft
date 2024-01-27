/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTextDocument>
#include "types.h"

namespace LC::Monocle
{
	class ResourcedTextDocument : public QTextDocument
	{
		const LazyImages_t Images_;

		QHash<QUrl, QSize> MaxImageSizes_;
	public:
		ResourcedTextDocument (const LazyImages_t& images);

		void SetMaxImageSizes (const QHash<QUrl, QSize>&);
	protected:
		QVariant loadResource (int type, const QUrl &name) override;
	};
}
