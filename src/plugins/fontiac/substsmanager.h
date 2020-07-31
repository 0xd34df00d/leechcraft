/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <QPair>
#include <QVariantList>
#include <QModelIndexList>

class QStandardItemModel;
class QAbstractItemModel;
class QFont;

namespace LC
{
namespace Fontiac
{
	class SubstsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;

		QList<QPair<QString, QString>> Substitutes_;
	public:
		SubstsManager (QObject *parent = nullptr);

		QAbstractItemModel* GetModel () const;
	private:
		void InitHeader ();

		void LoadSettings ();
		void SaveSettings () const;

		void AddItem (const QString&, const QString&, const QFont&);

		void RebuildSubsts (const QString&);
	private slots:
		void addRequested (const QString&, const QVariantList& datas);
		void modifyRequested (const QString&, int row, const QVariantList& datas);
		void removeRequested (const QString&, const QModelIndexList& rows);
	};
}
}
