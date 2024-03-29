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

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC::LMP::BrainSlugz
{
	class Checker;

	class ProgressModelManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::BrainSlugz::ProgressModelManager)

		QStandardItemModel * const Model_;
		QList<QStandardItem*> Row_;

		int InitialCount_ = 0;
	public:
		explicit ProgressModelManager (QObject* = nullptr);

		QAbstractItemModel* GetModel () const;

		void AddChecker (Checker*);
	};
}
