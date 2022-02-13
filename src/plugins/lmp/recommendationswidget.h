/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/media/audiostructs.h>
#include "ui_recommendationswidget.h"

namespace LC
{
namespace LMP
{
	class SimilarView;

	class RecommendationsWidget : public QWidget
	{
		Ui::RecommendationsWidget Ui_;
		SimilarView *RecView_ = nullptr;

		Media::SimilarityInfos_t Similars_;
	public:
		explicit RecommendationsWidget (QWidget* = nullptr);

		void InitializeProviders ();
	private:
		void HandleInfos (const Media::SimilarityInfos_t&);
	};
}
}
