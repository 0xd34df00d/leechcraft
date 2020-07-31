/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_PACKAGESDELEGATE_H
#define PLUGINS_LACKMAN_PACKAGESDELEGATE_H
#include <QStyledItemDelegate>
#include <QPointer>
#include <QHash>
#include <util/gui/selectablebrowser.h>

class QTreeView;
class QToolButton;

namespace LC
{
namespace LackMan
{
	class PackagesModel;

	class PackagesDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

		static const int CPadding;
		static const int CIconSize;
		static const int CActionsSize;
		static const int CTitleSizeDelta;
		static const int CNumLines;
	public:
		PackagesDelegate (QTreeView* = 0);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
	};
}
}

#endif
