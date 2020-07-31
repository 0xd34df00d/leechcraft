/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/media/audiostructs.h>
#include <interfaces/core/icoreproxyfwd.h>

class QQuickWidget;

class QStandardItemModel;

namespace LC
{
namespace LMP
{
	class SimilarViewManager : public QObject
	{
		QQuickWidget * const View_;
		QStandardItemModel * const Model_;
		const ICoreProxy_ptr Proxy_;
	public:
		SimilarViewManager (const ICoreProxy_ptr&, QQuickWidget*, QObject* = 0);

		void InitWithSource ();

		void DefaultRequest (const QString&);
		void SetInfos (Media::SimilarityInfos_t);
	};
}
}
