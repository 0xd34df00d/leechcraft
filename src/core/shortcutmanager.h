/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "interfaces/ihaveshortcuts.h"
#include "interfaces/core/ishortcutproxy.h"
#include "ui_shortcutmanager.h"

class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;

namespace LC
{
	class ShortcutManager : public QWidget
						  , public IShortcutProxy
	{
		Q_OBJECT
		Q_INTERFACES (IShortcutProxy)

		Ui::ShortcutManager Ui_;
		QStandardItemModel *Model_;
		QSortFilterProxyModel *Filter_;
	public:
		using IconsList_t = QVector<QPair<QStandardItem*, ActionInfo::Icon_t>>;
	private:
		IconsList_t Icons_;
	public:
		enum Roles
		{
			Object = Qt::UserRole + 1,
			OriginalName,
			Sequence,
			OldSequence
		};

		explicit ShortcutManager (QWidget* = nullptr);

		bool HasObject (QObject*) const override;

		void AddObject (QObject*);
		void AddObject (QObject*, const QString&, const QString&, const QIcon&);
		QList<QKeySequence> GetShortcuts (QObject*, const QByteArray&) override;
	private:
		int GetObjectRow (QObject*) const;
	public slots:
		void on_Tree__activated (const QModelIndex&);
		virtual void accept ();
		virtual void reject ();
	};
};
