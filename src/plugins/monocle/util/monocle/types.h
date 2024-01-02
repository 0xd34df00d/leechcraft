/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QImage>
#include <QString>
#include <QPair>
#include <QVector>

namespace LC::Monocle
{
	struct Span
	{
		int From_;
		int To_;
	};

	struct InternalLink
	{
		Span Link_;
		Span Target_;
		// Double-check Q_DECLARE_TYPEINFO when updating this type.
	};

	using LocatedImage_t = QPair<QString, QImage>;
	using ImagesList_t = QVector<LocatedImage_t>;
}

Q_DECLARE_TYPEINFO (LC::Monocle::Span, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO (LC::Monocle::InternalLink, Q_PRIMITIVE_TYPE);
