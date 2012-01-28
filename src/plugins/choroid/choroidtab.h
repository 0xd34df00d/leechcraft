/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
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

#ifndef PLUGINS_CHOROID_CHOROIDTAB_H
#define PLUGINS_CHOROID_CHOROIDTAB_H
#include <QWidget>
#include <QDeclarativeView>
#include <interfaces/ihavetabs.h>
#include "ui_choroidtab.h"

class QFileSystemModel;
class QStandardItemModel;
class QFileInfo;
class QDeclarativeView;

namespace LeechCraft
{
namespace Choroid
{
	class QMLItemModel;

	class ChoroidTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget);

		const TabClassInfo TabClass_;
		QObject *Parent_;

		Ui::ChoroidTab Ui_;

		QDeclarativeView *DeclView_;

		QMLItemModel *QMLFilesModel_;

		QFileSystemModel *FSModel_;
		QStandardItemModel *FilesModel_;

		enum CustomRoles
		{
			CRFilePath = 100
		};

		enum ImagesListRoles
		{
			ILRFilename = 100,
			ILRImage,
			ILRFileSize
		};
	public:
		ChoroidTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void LoadQML ();
		void ShowImage (const QString&);
	private slots:
		void handleDirTreeCurrentChanged (const QModelIndex&);
		void handleFileChanged (const QModelIndex&);
		void handleQMLImageSelected (const QString&);
		void handleStatusChanged (QDeclarativeView::Status);
	signals:
		void removeTab (QWidget*);
	};
}
}

#endif
