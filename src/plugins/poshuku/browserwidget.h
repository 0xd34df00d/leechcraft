/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_BROWSERWIDGET_H
#define PLUGINS_POSHUKU_BROWSERWIDGET_H
#include <boost/shared_ptr.hpp>
#include <QWidget>
#include <QTime>
#include <interfaces/imultitabs.h>
#include <interfaces/iwebbrowser.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/structures.h>
#include "ui_browserwidget.h"

class QToolBar;
class QDataStream;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FindDialog;
			class PasswordRemember;
			struct BrowserWidgetSettings;

			class BrowserWidget : public QWidget
								, public IWebWidget
								, public IMultiTabsWidget
			{
				Q_OBJECT
				Q_INTERFACES (IWebWidget IMultiTabsWidget)

				Ui::BrowserWidget Ui_;

				QToolBar *ToolBar_;
				QAction *Add2Favorites_;
				QAction *Find_;
				QAction *Print_;
				QAction *PrintPreview_;
				QAction *ScreenSave_;
				QAction *ViewSources_;
				QAction *ZoomIn_;
				QAction *ZoomOut_;
				QAction *ZoomReset_;
				QAction *Cut_;
				QAction *Copy_;
				QAction *Paste_;
				QAction *Back_;
				QAction *Forward_;
				QAction *Reload_;
				QAction *Stop_;
				QAction *ReloadStop_;
				QAction *ReloadPeriodically_;
				QAction *NotifyWhenFinished_;
				QAction *RecentlyClosedAction_;
				QPoint OnLoadPos_;
				QMenu *ChangeEncoding_;
				QMenu *RecentlyClosed_;
				QMenu *ExternalLinks_;
				FindDialog *FindDialog_;
				PasswordRemember *RememberDialog_;
				QTimer *ReloadTimer_;
				QString PreviousFindText_;
				bool HtmlMode_;
				bool Own_;

				static QObject* S_MultiTabsParent_;

				friend class CustomWebView;
			public:
				enum Actions
				{
					EAAdd2Favorites_,
					EAFind_,
					EAPrint_,
					EAPrintPreview_,
					EAScreenSave_,
					EAViewSources_,
					EAUnused0,
					EAUnused1,
					EAZoomIn_,
					EAZoomOut_,
					EAZoomReset_,
					EAUnused2,
					EAUnused3,
					EACut_,
					EACopy_,
					EAPaste_,
					EABack_,
					EAForward_,
					EAReload_,
					EAStop_,
					EARecentlyClosedAction_,
					ActionMax = 2000
				};

				BrowserWidget (QWidget* = 0);
				virtual ~BrowserWidget ();
				static void SetParentMultiTabs (QObject*);

				void Deown ();
				void InitShortcuts ();

				void SetUnclosers (const QList<QAction*>&);
				CustomWebView* GetView () const;
				BrowserWidgetSettings GetWidgetSettings () const;
				void SetWidgetSettings (const BrowserWidgetSettings&);
				void SetURL (const QUrl&);

				void Load (const QString&);
				void SetHtml (const QString&, const QUrl& = QUrl ());
				void SetNavBarVisible (bool);
				QWidget* Widget ();

				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;

				void Remove ();
				QToolBar* GetToolBar () const;
				void NewTabRequested ();
				QList<QAction*> GetTabBarContextMenuActions () const;
				QObject* ParentMultiTabs () const;

				void SetOnLoadScrollPoint (const QPoint&);
			private:
				void PrintImpl (bool, QWebFrame*);
				void SetActualReloadInterval (const QTime&);
			public slots:
				void notificationActionTriggered (int);
			private slots:
				void handleIconChanged ();
				void handleStatusBarMessage (const QString&);
				void on_URLEdit__returnPressed ();
				void handleReloadPeriodically ();
				void handleAdd2Favorites ();
				void handleFind ();
				void findText (const QString&, QWebPage::FindFlags);
				void handleViewPrint (QWebFrame*);
				void handlePrinting ();
				void handlePrintingWithPreview ();
				void handleScreenSave ();
				void handleViewSources ();
				void focusLineEdit ();
				void handleNewUnclose (QAction*);
				void handleUncloseDestroyed ();
				void updateTooltip ();
				void enableActions ();
				void handleEntityAction ();
				void checkLinkRels ();
				void setScrollPosition ();
				void pageFocus ();
				void handleLoadProgress (int);
				void notifyLoadFinished (bool);
				void handleChangeEncodingAboutToShow ();
				void handleChangeEncodingTriggered (QAction*);
				void updateLogicalPath ();
				void showSendersMenu ();
			signals:
				void titleChanged (const QString&);
				void urlChanged (const QString&);
				void iconChanged (const QIcon&);
				void needToClose ();
				void tooltipChanged (QWidget*);
				void addToFavorites (const QString&, const QString&);
				void statusBarChanged (const QString&);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void couldHandle (const LeechCraft::DownloadEntity&, bool*);
				void invalidateSettings ();
				void raiseTab (QWidget*);
			};
		};
	};
};

#endif

