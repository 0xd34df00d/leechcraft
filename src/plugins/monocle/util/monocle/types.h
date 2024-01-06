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

	using LocatedImage_t = QPair<QString, QImage>;
	using ImagesList_t = QVector<LocatedImage_t>;
}

template<>
class QTypeInfo<LC::Monocle::Span> : public QTypeInfo<int> {};

template<>
class QTypeInfo<LC::Monocle::InternalLink> : public QTypeInfoMerger<QString, LC::Monocle::Span> {};
