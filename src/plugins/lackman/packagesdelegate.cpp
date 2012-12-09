/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

namespace LeechCraft
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
