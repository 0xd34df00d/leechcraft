/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QTreeView>

namespace LeechCraft
{
namespace NetStoreManager
{
	class FilesView : public QTreeView
	{
		Q_OBJECT

		QAction *MoveItem_;
		QAction *CopyItem_;
		QAction *Cancel_;

		QDropEvent *CurrentEvent_;
		QByteArray DraggedItemId_;
		QByteArray TargetItemId_;
	public:
		FilesView (QWidget *parent = 0);

	protected:
		void dropEvent (QDropEvent *event);

	private slots:
		void handleCopyItem ();
		void handleMoveItem ();
		void handleCancel ();

	signals:
		void itemAboutToBeCopied (const QByteArray& itemId, const QByteArray&  newParentId);
		void itemAboutToBeMoved (const QByteArray&  itemId, const QByteArray&  newParentId);
		void itemAboutToBeRestoredFromTrash (const QByteArray&  itemId);
		void itemAboutToBeTrashed (const QByteArray&  itemId);
	};
}
}
