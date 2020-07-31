/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "packagesdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QTreeView>
#include <QAction>
#include <QToolButton>
#include <QTimer>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMainWindow>
#include <QAbstractProxyModel>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "packagesmodel.h"
#include "core.h"
#include "pendingmanager.h"
#include "delegatebuttongroup.h"

namespace LC
{
namespace LackMan
{
	const int PackagesDelegate::CPadding = 5;
	const int PackagesDelegate::CIconSize = 32;
	const int PackagesDelegate::CActionsSize = 32;
	const int PackagesDelegate::CTitleSizeDelta = 2;

	PackagesDelegate::PackagesDelegate (QTreeView *parent)
	: QStyledItemDelegate (parent)
	{
	}

	void PackagesDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QStyledItemDelegate::paint (painter, option, index);
	}
}
}
