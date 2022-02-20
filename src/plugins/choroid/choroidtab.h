/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QWidget>
#include <QUrl>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_choroidtab.h"

class QFileSystemModel;
class QStandardItemModel;
class QStandardItem;
class QFileInfo;
class QDeclarativeView;

class QQuickWidget;

namespace LC
{
namespace Choroid
{
	class QMLItemModel;

	class ChoroidTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		const TabClassInfo TabClass_;
		QObject *Parent_;
		ICoreProxy_ptr Proxy_;

		Ui::ChoroidTab Ui_;

		QQuickWidget *DeclView_;
		QMLItemModel *QMLFilesModel_;

		QFileSystemModel *FSModel_;
		QStandardItemModel *FilesModel_;

		QUrl CurrentImage_;

		QToolBar *Bar_;
		QMenu *SortMenu_;

		enum CustomRoles
		{
			CRFilePath = Qt::UserRole + 1
		};

		std::function<bool (QFileInfo, QFileInfo)> CurrentSorter_;
	public:
		ChoroidTab (const TabClassInfo&, ICoreProxy_ptr, QObject*);
		~ChoroidTab ();

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void LoadQML ();
		void SetSortMenu ();
		void ShowImage (const QString&);
		void ShowImage (const QUrl&);
		QStandardItem* FindFileItem (const QString&);
	private slots:
		void sortByName ();
		void sortByDate ();
		void sortBySize ();
		void sortByNumber ();

		void reload ();

		void handleDirTreeCurrentChanged (const QModelIndex&);
		void handleFileChanged (const QModelIndex&);

		void handleQMLImageSelected (const QString&);
		void showNextImage ();
		void showPrevImage ();
		void goUp ();
	signals:
		void removeTab ();
	};
}
}
