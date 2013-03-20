/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QWidget>
#include <QComboBox>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/idndtab.h>
#include "interfaces/monocle/idocument.h"
#include "ui_documenttab.h"

class QDockWidget;

namespace LeechCraft
{
namespace Monocle
{
	enum class LayoutMode;

	class PagesLayoutManager;
	class PageGraphicsItem;
	class TOCWidget;
	class BookmarksWidget;
	class ThumbsWidget;

	class DocumentTab : public QWidget
					  , public ITabWidget
					  , public IRecoverableTab
					  , public IDNDTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab IDNDTab)

		Ui::DocumentTab Ui_;

		TabClassInfo TC_;
		QObject *ParentPlugin_;

		QToolBar *Toolbar_;
		QComboBox *ScalesBox_;
		QAction *ZoomOut_;
		QAction *ZoomIn_;
		QLineEdit *PageNumLabel_;

		QAction *LayOnePage_;
		QAction *LayTwoPages_;

		PagesLayoutManager *LayoutManager_;

		QDockWidget *DockWidget_;
		TOCWidget *TOCWidget_;
		BookmarksWidget *BMWidget_;
		ThumbsWidget *ThumbsWidget_;

		IDocument_ptr CurrentDoc_;
		QString CurrentDocPath_;
		QList<PageGraphicsItem*> Pages_;
		QGraphicsScene Scene_;

		enum class MouseMode
		{
			Move,
			Select
		} MouseMode_;

		bool SaveStateScheduled_;

		int PrevCurrentPage_;

		struct OnloadData
		{
			int Num_;
			double X_;
			double Y_;
		} Onload_;
	public:
		DocumentTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;
		QByteArray GetTabRecoverData () const;

		void FillMimeData (QMimeData*);
		void HandleDragEnter (QDragMoveEvent*);
		void HandleDrop (QDropEvent*);

		void RecoverState (const QByteArray&);

		void ReloadDoc (const QString&);
		bool SetDoc (const QString&);

		void CreateViewCtxMenuActions (QMenu*);

		int GetCurrentPage () const;
		void SetCurrentPage (int, bool immediate = false);

		QPoint GetCurrentCenter () const;
		void CenterOn (const QPoint&);
	private:
		void SetupToolbar ();

		QPoint GetViewportCenter () const;
		void Relayout ();

		QImage GetSelectionImg ();
		QString GetSelectionText () const;

		void RegenPageVisibility ();
	private slots:
		void handleNavigateRequested (QString, int, double, double);
		void handleThumbnailClicked (int);

		void handlePageContentsChanged (int);

		void scheduleSaveState ();
		void saveState ();

		void handleRecentOpenAction (QAction*);

		void selectFile ();
		void handlePrint ();
		void handlePresentation ();

		void handleGoPrev ();
		void handleGoNext ();
		void navigateNumLabel ();
		void updateNumLabel ();
		void checkCurrentPageChange (bool force = false);

		void zoomOut ();
		void zoomIn ();

		void showOnePage ();
		void showTwoPages ();
		void syncUIToLayMode ();

		void setMoveMode (bool);
		void setSelectionMode (bool);

		void handleCopyAsImage ();
		void handleSaveAsImage ();
		void handleCopyAsText ();

		void showDocInfo ();

		void delayedCenterOn (const QPoint&);

		void handleScaleChosen (int);

		void handleDockLocation (Qt::DockWidgetArea);
	signals:
		void changeTabName (QWidget*, const QString&);
		void removeTab (QWidget*);

		void tabRecoverDataChanged ();

		void fileLoaded (const QString&);

		void currentPageChanged (int);
		void pagesVisibilityChanged (const QMap<int, QRect>&);
	};
}
}
