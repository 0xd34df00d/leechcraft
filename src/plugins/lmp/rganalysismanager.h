/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include "interfaces/lmp/collectiontypes.h"

namespace LC
{
namespace LMP
{
	class RgAnalyser;
	class LocalCollection;

	class RgAnalysisManager : public QObject
	{
		Q_OBJECT

		LocalCollection * const Coll_;

		std::shared_ptr<RgAnalyser> CurrentAnalyser_;

		QList<Collection::Album_ptr> AlbumsQueue_;
	public:
		RgAnalysisManager (LocalCollection *coll, QObject* = nullptr);
	private slots:
		void handleAnalysed ();
		void rotateQueue ();
	public slots:
		void handleScanFinished ();
	};
}
}
