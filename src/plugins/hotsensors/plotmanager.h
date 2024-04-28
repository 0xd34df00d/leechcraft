/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "structures.h"

class QAbstractItemModel;
class QStandardItemModel;

namespace LC
{
namespace HotSensors
{
	class PlotManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
	public:
		PlotManager (QObject* = 0);

		QAbstractItemModel* GetModel () const;

		std::unique_ptr<QObject> CreateContextWrapper ();
	public slots:
		void handleHistoryUpdated (const ReadingsHistory_t&);
	};
}
}
