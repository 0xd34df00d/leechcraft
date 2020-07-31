/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTreeView>

namespace LC
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
		QList<QByteArray> DraggedItemsIds_;
		QByteArray TargetItemId_;
	public:
		FilesView (QWidget *parent = 0);

	protected:
		void dropEvent (QDropEvent *event);
		void keyReleaseEvent (QKeyEvent *event);

	private slots:
		void handleCopyItem ();
		void handleMoveItem ();
		void handleCancel ();

	signals:
		void itemsAboutToBeCopied (const QList<QByteArray>& ids,
				const QByteArray& newParentId);
		void itemsAboutToBeMoved (const QList<QByteArray>& ids,
				const QByteArray& newParentId);
		void itemsAboutToBeRestoredFromTrash (const QList<QByteArray>& ids);
		void itemsAboutToBeTrashed (const QList<QByteArray>& ids);

		void returnPressed ();
		void backspacePressed ();
		void quoteLeftPressed ();
	};
}
}
