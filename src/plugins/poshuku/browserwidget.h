/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <QTime>
#include <QMenu>
#include <interfaces/ihavetabs.h>
#include <interfaces/idndtab.h>
#include <interfaces/iwebbrowser.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/structures.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/iwkfontssettable.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/poshuku/poshukutypes.h>
#include "interfaces/poshuku/ibrowserwidget.h"
#include "interfaces/poshuku/iwebview.h"
#include "ui_browserwidget.h"

class QToolBar;
class QDataStream;
class QShortcut;
class QLabel;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Poshuku
{
	class PasswordRemember;
	struct BrowserWidgetSettings;

	class BrowserWidget : public QWidget
						, public IBrowserWidget
						, public IWebWidget
						, public ITabWidget
						, public IDNDTab
						, public IRecoverableTab
						, public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IBrowserWidget
				IWebWidget
				ITabWidget
				IDNDTab
				IRecoverableTab
				IWkFontsSettable)

		Ui::BrowserWidget Ui_;

		QToolBar *ToolBar_;
		QAction *Add2Favorites_;
		QAction *Find_;
		QAction *Print_;
		QAction *PrintPreview_;
		QAction *ScreenSave_;
		QAction *ViewSources_;
		QAction *SavePage_;

		QAction *ZoomIn_;
		QAction *ZoomOut_;
		QAction *ZoomReset_;

		QAction *Cut_;
		QAction *Copy_;
		QAction *Paste_;
		QAction *Back_;
		QMenu *BackMenu_;
		QAction *Forward_;
		QMenu *ForwardMenu_;
		QAction *Reload_;
		QAction *Stop_;
		QAction *ReloadStop_;
		QAction *ReloadPeriodically_;
		QAction *NotifyWhenFinished_;
		QAction *HistoryAction_;
		QAction *BookmarksAction_;
		QPoint OnLoadPos_;
		QMenu *ChangeEncoding_;
		PasswordRemember *RememberDialog_;
		QTimer *ReloadTimer_;
		bool HtmlMode_ = false;
		bool Own_ = true;
		QMap<QString, QList<QAction*>> WindowMenus_;

		const IWebView_ptr WebView_;
		const ICoreProxy_ptr Proxy_;

		QLabel *LinkTextItem_;

		static QObject* S_MultiTabsParent_;
	public:
		BrowserWidget (const IWebView_ptr&, Util::ShortcutManager*, const ICoreProxy_ptr&, QWidget* = nullptr);
		virtual ~BrowserWidget ();
		static void SetParentMultiTabs (QObject*);

		void Deown ();
		void FinalizeInit ();

		QLineEdit* GetURLEdit () const override;
		IWebView* GetWebView () const override;
		void InsertFindAction (QMenu*, const QString&) override;
		void AddStandardActions (QMenu*) override;

		QObject* GetQObject () override;

		BrowserWidgetSettings GetWidgetSettings () const;
		void SetWidgetSettings (const BrowserWidgetSettings&);
		void SetURL (const QUrl&);

		void Load (const QUrl&) override;
		void SetHtml (const QString&, const QUrl& = QUrl ()) override;
		void SetNavBarVisible (bool) override;
		void SetEverythingElseVisible (bool) override;
		QWidget* GetQWidget () override;

		void Remove () override;
		QToolBar* GetToolBar () const override;
		QList<QAction*> GetTabBarContextMenuActions () const override;
		QMap<QString, QList<QAction*>> GetWindowMenus () const override;
		QObject* ParentMultiTabs () override;
		TabClassInfo GetTabClassInfo () const override;

		void FillMimeData (QMimeData*) override;
		void HandleDragEnter (QDragMoveEvent*) override;
		void HandleDrop (QDropEvent*) override;

		void SetTabRecoverData (const QByteArray&);
		QByteArray GetTabRecoverData () const override;
		QString GetTabRecoverName () const override;
		QIcon GetTabRecoverIcon () const override;

		void SetFontFamily (FontFamily family, const QFont& font) override;
		void SetFontSize (FontSize type, int size) override;

		void SetOnLoadScrollPoint (const QPoint&);
	protected:
		void keyReleaseEvent (QKeyEvent*) override;
	private:
		void SetActualReloadInterval (const QTime&);
		void SetSplitterSizes (int);
		void RegisterShortcuts (Util::ShortcutManager*);
	public slots:
		void focusLineEdit ();
		void handleShortcutHistory ();
		void handleShortcutBookmarks ();
		QLineEdit* getAddressBar () const;
		QWidget* getSideBar () const;
	private slots:
		void handleIconChanged ();
		void handleStatusBarMessage (const QString&);
		void handleURLFrameLoad (QString);
		void handleReloadPeriodically ();
		void handleAdd2Favorites ();
		void handleFind ();
		void handleScreenSave ();
		void handleViewSources ();
		void handleSavePage ();
		void enableActions ();

		void updateTitle (QString);

		void updateNavHistory ();

		void checkLoadedDocument ();

		void handleContextMenu (const QPoint&, const ContextMenuInfo&);

		void setScrollPosition ();
		void pageFocus ();
		void handleLoadProgress (int);
		void notifyLoadFinished (bool);
		void handleChangeEncodingAboutToShow ();
		void handleChangeEncodingTriggered (QAction*);
		void updateLogicalPath ();
		void handleUrlChanged (const QUrl&);

		void handleFeaturePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr&, IWebView::Feature);
	signals:
		void titleChanged (const QString&);
		void urlChanged (const QUrl&) override;
		void iconChanged (const QIcon&);
		void needToClose ();
		void tooltipChanged (QWidget*);
		void addToFavorites (const QString&, const QString&);
		void statusBarChanged (const QString&);
		void raiseTab (QWidget*);
		void tabRecoverDataChanged () override;

		// Hook support
		void hookBrowserWidgetInitialized (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookIconChanged (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookLoadProgress (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				int progress);
		void hookMoreMenuFillBegin (LC::IHookProxy_ptr proxy,
				QMenu *menu,
				QObject *browserWidget);
		void hookMoreMenuFillEnd (LC::IHookProxy_ptr proxy,
				QMenu *menu,
				QObject *browserWidget);
		void hookNotifyLoadFinished (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				bool ok,
				bool notifyWhenFinished,
				bool own,
				bool htmlMode);
		void hookSetURL (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QUrl url);
		void hookStatusBarMessage (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString message);
		void hookTabBarContextMenuActions (LC::IHookProxy_ptr proxy,
				const QObject *browserWidget) const;
		void hookTabRemoveRequested (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookUpdateLogicalPath (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookURLEditReturnPressed (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookWebViewContextMenu (LC::IHookProxy_ptr,
				LC::Poshuku::IWebView*,
				const LC::Poshuku::ContextMenuInfo& hitTestResult,
				QMenu*,
				WebViewCtxMenuStage);
	};
}
}
