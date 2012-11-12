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

#pragma once

#include <QWidget>
#include <QComboBox>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "interfaces/monocle/idocument.h"
#include "ui_documenttab.h"

class QDockWidget;

namespace LeechCraft
{
namespace Monocle
{
	class PageGraphicsItem;
	class TOCWidget;

	class DocumentTab : public QWidget
					  , public ITabWidget
					  , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::DocumentTab Ui_;

		TabClassInfo TC_;
		QObject *ParentPlugin_;

		QToolBar *Toolbar_;
		QComboBox *ScalesBox_;
		QLineEdit *PageNumLabel_;

		QDockWidget *DockTOC_;
		TOCWidget *TOCWidget_;

		IDocument_ptr CurrentDoc_;
		QString CurrentDocPath_;
		QList<PageGraphicsItem*> Pages_;
		QGraphicsScene Scene_;

		enum class LayoutMode
		{
			OnePage,
			TwoPages
		} LayMode_;

		enum class MouseMode
		{
			Move,
			Select
		} MouseMode_;

		bool RelayoutScheduled_;

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

		void RecoverState (const QByteArray&);

		void ReloadDoc (const QString&);

		bool SetDoc (const QString&);
	private:
		void SetupToolbar ();

		double GetCurrentScale () const;

		QPoint GetViewportCenter () const;
		int GetCurrentPage () const;
		void SetCurrentPage (int);
		void Relayout (double);

		void ClearViewActions ();
	private slots:
		void handleNavigateRequested (QString, int, double, double);

		void handlePageSizeChanged (int);
		void handlePageContentsChanged (int);

		void scheduleRelayout ();
		void handleRelayout ();

		void handleRecentOpenAction (QAction*);

		void selectFile ();
		void handlePrint ();
		void handlePresentation ();

		void handleGoPrev ();
		void handleGoNext ();
		void navigateNumLabel ();
		void updateNumLabel ();

		void showOnePage ();
		void showTwoPages ();

		void setMoveMode (bool);
		void setSelectionMode (bool);

		void handleCopyAsImage ();
		void handleCopyAsText ();

		void delayedCenterOn (const QPoint&);

		void handleScaleChosen (int);
	signals:
		void changeTabName (QWidget*, const QString&);
		void removeTab (QWidget*);

		void tabRecoverDataChanged ();

		void fileLoaded (const QString&);
	};
}
}
