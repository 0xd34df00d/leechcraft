/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantList>
#include <QModelIndexList>
#include "interfaces/azoth/azothcommon.h"

class QAbstractItemModel;
class QStandardItemModel;

namespace LC
{
namespace Azoth
{
	class CustomStatusesManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
	public:
		CustomStatusesManager (QObject* = 0);

		QAbstractItemModel* GetModel () const;
		QList<CustomStatus> GetStates () const;
	private:
		void Save ();
		void Load ();

		void Add (const CustomStatus&, int = -1);
		CustomStatus GetCustom (int) const;
	public slots:
		void addRequested (const QString&, const QVariantList&);
		void modifyRequested (const QString&, int, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);
	};
}
}
