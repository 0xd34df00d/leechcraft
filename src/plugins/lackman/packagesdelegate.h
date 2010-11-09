/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_LACKMAN_PACKAGESDELEGATE_H
#define PLUGINS_LACKMAN_PACKAGESDELEGATE_H
#include <QStyledItemDelegate>
#include <QPointer>
#include <QHash>
#include <plugininterface/selectablebrowser.h>

class QTreeView;
class QToolButton;

namespace LeechCraft
{
	namespace Plugins
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

				mutable QModelIndex CurrentSelection_;
				mutable QPointer<Util::SelectableBrowser> SelectableBrowser_;
				mutable QHash<int, QToolButton*> Row2InstallRemove_;
				mutable QHash<int, QToolButton*> Row2Update_;
				mutable QHash<int, QWidget*> Row2Layout_;
				mutable QHash<QModelIndex, bool> WasInstalled_;
				mutable QHash<QModelIndex, bool> WasUpgradable_;

				QWidget * const Viewport_;
				PackagesModel * const Model_;
			public:
				PackagesDelegate (QTreeView* = 0);

				void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
				QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const;
			private:
				int TitleHeight (const QStyleOptionViewItem&) const;
				int TextHeight (const QStyleOptionViewItem&) const;
				int CurrentInfoHeight (const QStyleOptionViewItem&) const;
				void PrepareSelectableBrowser () const;
				QToolButton* GetInstallRemove (const QModelIndex&) const;
				QToolButton* GetUpdate (const QModelIndex&) const;
				QWidget* GetLayout (const QModelIndex&) const;
			public slots:
				void handleRowChanged (const QModelIndex&, const QModelIndex&);
				void invalidateWidgetPositions ();
				void hideOverflousActions (const QModelIndex&, int, int);
			private slots:
				void handleAction ();
				void handleRowActionFinished (int);
				void handlePackageUpdateToggled (int, bool);
			};
		}
	}
}

#endif
