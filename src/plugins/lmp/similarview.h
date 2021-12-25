/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QQuickWidget>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace LMP
{
	class SimilarViewManager;

	class SimilarView : public QQuickWidget
	{
		Q_OBJECT

		SimilarViewManager *Manager_;
	public:
		SimilarView (QWidget* = nullptr);

		void SetSimilarArtists (Media::SimilarityInfos_t);
	};
}
}
