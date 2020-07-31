/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QMutex>
#include "progresslistener.h"

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Snails
{
	class Account;

	class ProgressManager : public QObject
	{
		QStandardItemModel *Model_;

		QMutex Listener2RowMutex_;
		QMap<ProgressListener_wptr, QList<QStandardItem*>> Listener2Row_;
	public:
		ProgressManager (QObject* = nullptr);

		QAbstractItemModel* GetRepresentation () const;

		ProgressListener_ptr MakeProgressListener (const QString&);
	};
}
}
