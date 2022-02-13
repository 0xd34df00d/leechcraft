/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>
#include <interfaces/media/audiostructs.h>
#include "similarmodel.h"

class QQuickWidget;

class QStandardItemModel;

namespace LC
{
namespace LMP
{
	class SimilarViewManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::SimilarViewManager)

		QQuickWidget * const View_;
		SimilarModel * const Model_;
	public:
		explicit SimilarViewManager (QQuickWidget*, QObject* = nullptr);

		void DefaultRequest (const QString&);
		void SetInfos (Media::SimilarityInfos_t);
	};
}
}
