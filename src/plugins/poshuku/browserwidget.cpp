/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "browserwidget.h"

#ifdef ENABLE_IDN
#include <idna.h>
#endif

#include <boost/bind.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QtDebug>
#include <QToolBar>
#include <QToolButton>
#include <QBuffer>
#include <QDial>
#include <QMenu>
#include <QMovie>
#include <QWidgetAction>
#include <QPrinter>
#include <QPrintDialog>
#include <QTimer>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <qwebframe.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextCodec>
#include <QCursor>
#include <qwebhistory.h>
#include <qwebelement.h>
#include <QDataStream>
#include <QRegExp>
#include <QKeySequence>
#include <QGraphicsScene>
#include <QGraphicsOpacityEffect>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include <util/notificationactionhandler.h>
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "historymodel.h"
#include "finddialog.h"
#include "screenshotsavedialog.h"
#include "xmlsettingsmanager.h"
#include "sourceviewer.h"
#include "passwordremember.h"
#include "reloadintervalselector.h"
#include "browserwidgetsettings.h"
#include "bookmarkswidget.h"
#include "historywidget.h"
#include "customwebview.h"

Q_DECLARE_METATYPE (QList<QObject*>);

namespace LeechCraft
{
namespace Poshuku
{
	using LeechCraft::ActionInfo;

	QObject *BrowserWidget::S_MultiTabsParent_ = 0;

	namespace
	{
		class ResizeEventFilter : public QObject
		{
		public:
			ResizeEventFilter (QObject *parent = 0)
			: QObject (parent)
			{
			}
		protected:
			bool eventFilter (QObject *obj, QEvent *e)
			{
				if (e->type () == QEvent::Resize)
					QMetaObject::invokeMethod (parent (),
							"refitWebView", Qt::QueuedConnection);

				return QObject::eventFilter (obj, e);
			}
		};
	}

	BrowserWidget::BrowserWidget (QWidget *parent)
	: QWidget (parent)
	, ReloadTimer_ (new QTimer (this))
	, HtmlMode_ (false)
	, Own_ (true)
	{
		Ui_.setupUi (this);
		Core::Instance ().GetPluginManager ()->RegisterHookable (this);
		Ui_.Sidebar_->AddPage (tr ("Bookmarks"), new BookmarksWidget);
		Ui_.Sidebar_->AddPage (tr ("History"), new HistoryWidget);
		Ui_.Splitter_->setSizes (QList<int> () << 0 << 1000);
		Ui_.Progress_->hide ();

		WebView_ = new CustomWebView;

		QGraphicsScene *scene = new QGraphicsScene (this);
		scene->addItem (WebView_);

		Ui_.WebGraphicsView_->setScene (scene);
		QTimer::singleShot (0,
				this,
				SLOT (refitWebView ()));

		Ui_.WebGraphicsView_->installEventFilter (new ResizeEventFilter (this));

		WebView_->SetBrowserWidget (this);
		connect (WebView_,
				SIGNAL (invalidateSettings ()),
				this,
				SIGNAL (invalidateSettings ()));

		connect (ReloadTimer_,
				SIGNAL (timeout ()),
				WebView_,
				SLOT (reload ()));

		Cut_ = WebView_->pageAction (QWebPage::Cut);
		Cut_->setShortcutContext (Qt::WindowShortcut);
		Cut_->setProperty ("ActionIcon", "poshuku_cut");
		Copy_ = WebView_->pageAction (QWebPage::Copy);
		Copy_->setShortcutContext (Qt::WindowShortcut);
		Copy_->setProperty ("ActionIcon", "poshuku_copy");
		Paste_ = WebView_->pageAction (QWebPage::Paste);
		Paste_->setProperty ("ActionIcon", "poshuku_paste");

		ToolBar_ = new QToolBar (this);
		ToolBar_->setWindowTitle ("Poshuku");

		Back_ = WebView_->pageAction (QWebPage::Back);
		Back_->setParent (this);
		Back_->setProperty ("ActionIcon", "poshuku_back");

		Forward_ = WebView_->pageAction (QWebPage::Forward);
		Forward_->setParent (this);
		Forward_->setProperty ("ActionIcon", "poshuku_forward");

		Reload_ = WebView_->pageAction (QWebPage::Reload);
		Reload_->setProperty ("ActionIcon", "poshuku_reload");
		Reload_->setIcon (Core::Instance ()
				.GetProxy ()->GetIcon ("poshuku_reload"));

		Stop_ = WebView_->pageAction (QWebPage::Stop);
		Stop_->setProperty ("ActionIcon", "poshuku_stop");
		Stop_->setIcon (Core::Instance ()
				.GetProxy ()->GetIcon ("poshuku_stop"));

		ReloadStop_ = new QAction (this);
		handleLoadProgress (0);

		ReloadPeriodically_ = new QAction (tr ("Reload periodically"), this);
		ReloadPeriodically_->setCheckable (true);
		ReloadPeriodically_->setProperty ("ActionIcon", "poshuku_reloadperiodically");

		NotifyWhenFinished_ = new QAction (tr ("Notify when finished loading"), this);
		NotifyWhenFinished_->setCheckable (true);
		NotifyWhenFinished_->setProperty ("ActionIcon", "poshuku_notifywhenfinished");
		NotifyWhenFinished_->setChecked (XmlSettingsManager::Instance ()->
				property ("NotifyFinishedByDefault").toBool ());


		Add2Favorites_ = new QAction (tr ("Bookmark..."), this);
		Add2Favorites_->setProperty ("ActionIcon", "poshuku_addbookmark");
		Add2Favorites_->setEnabled (false);

		IAddressBar *iab = qobject_cast<IAddressBar*> (GetURLEdit ());
		if (!iab)
			qWarning () << Q_FUNC_INFO
					<< GetURLEdit ()
					<< "isn't an IAddressBar object";
		else
		{
			iab->InsertAction (Add2Favorites_, 0, true);
			connect (GetURLEdit (),
					SIGNAL (textChanged (const QString&)),
					this,
					SLOT (handleUrlTextChanged (const QString&)));

			connect (&Core::Instance (),
					SIGNAL (bookmarkAdded (const QString&)),
					this,
					SLOT (checkPageAsFavorite (const QString&)));

			connect (&Core::Instance (),
					SIGNAL (bookmarkRemoved (const QString&)),
					this,
					SLOT (checkPageAsFavorite (const QString&)));
		}

		Find_ = new QAction (tr ("Find..."),
				this);
		Find_->setProperty ("ActionIcon", "poshuku_find");
		Find_->setEnabled (false);

		Print_ = new QAction (tr ("Print..."),
				this);
		Print_->setProperty ("ActionIcon", "poshuku_print");
		Print_->setEnabled (false);

		PrintPreview_ = new QAction (tr ("Print with preview..."),
				this);
		PrintPreview_->setProperty ("ActionIcon", "poshuku_printpreview");
		PrintPreview_->setEnabled (false);

		ScreenSave_ = new QAction (tr ("Take page's screenshot..."),
				this);
		ScreenSave_->setProperty ("ActionIcon", "poshuku_takescreenshot");
		ScreenSave_->setEnabled (false);

		ViewSources_ = new QAction (tr ("View sources..."),
				this);
		ViewSources_->setProperty ("ActionIcon", "poshuku_viewsources");
		ViewSources_->setEnabled (false);

		SavePage_ = new QAction (tr ("Save page..."),
				this);
		SavePage_->setProperty ("ActionIcon", "fetchall");
		SavePage_->setEnabled (false);
		SavePage_->setShortcut (QKeySequence (tr ("Ctrl+s")));

		ZoomIn_ = new QAction (tr ("Zoom in"),
				this);
		ZoomIn_->setProperty ("ActionIcon", "poshuku_zoomin");

		ZoomOut_ = new QAction (tr ("Zoom out"),
				this);
		ZoomOut_->setProperty ("ActionIcon", "poshuku_zoomout");

		ZoomReset_ = new QAction (tr ("Reset zoom"),
				this);
		ZoomReset_->setProperty ("ActionIcon", "poshuku_zoomreset");

		HistoryAction_ = new QAction (tr ("Open history"),
				this);
		HistoryAction_->setCheckable (true);
		HistoryAction_->setShortcut (QKeySequence (tr ("Ctrl+h")));

		BookmarksAction_ = new QAction (tr ("Open bookmarks"),
				this);
		BookmarksAction_->setCheckable (true);
		BookmarksAction_->setShortcut (QKeySequence (tr ("Ctrl+b")));


		ToolBar_->addAction (Back_);
		ToolBar_->addAction (Forward_);
		ToolBar_->addAction (ReloadStop_);

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy ());
		QMenu *moreMenu = new QMenu (this);
		emit hookMoreMenuFillBegin (proxy, moreMenu, WebView_, this);
		if (!proxy->IsCancelled ())
		{
			const QString tools = "Poshuku";
			WindowMenus_ [tools] << Find_;
			WindowMenus_ [tools] << Add2Favorites_;
			WindowMenus_ [tools] << Util::CreateSeparator (this);
			WindowMenus_ [tools] << HistoryAction_;
			WindowMenus_ [tools] << BookmarksAction_;
			WindowMenus_ [tools] << Util::CreateSeparator (this);
			WindowMenus_ [tools] << ReloadPeriodically_;
			WindowMenus_ [tools] << NotifyWhenFinished_;
			WindowMenus_ [tools] << Util::CreateSeparator (this);
			WindowMenus_ [tools] << Print_;
			WindowMenus_ [tools] << PrintPreview_;
			WindowMenus_ [tools] << ScreenSave_;
			WindowMenus_ [tools] << Util::CreateSeparator (this);
			WindowMenus_ [tools] << ViewSources_;
			WindowMenus_ [tools] << SavePage_;
			WindowMenus_ [tools] << Util::CreateSeparator (this);

			const QString view = "view";
			WindowMenus_ [view] << ZoomIn_;
			WindowMenus_ [view] << ZoomOut_;
			WindowMenus_ [view] << ZoomReset_;
			WindowMenus_ [view] << Util::CreateSeparator (this);
		}
		proxy.reset (new Util::DefaultHookProxy ());
		emit hookMoreMenuFillEnd (proxy, moreMenu, WebView_, this);

		if (moreMenu->actions ().size ())
		{
			const QString tools = "Poshuku";
			WindowMenus_ [tools] << moreMenu->actions ();
			WindowMenus_ [tools] << Util::CreateSeparator (this);
		}

		ChangeEncoding_ = moreMenu->addMenu (tr ("Change encoding"));
		connect (ChangeEncoding_,
				SIGNAL (aboutToShow ()),
				this,
				SLOT (handleChangeEncodingAboutToShow ()));
		connect (ChangeEncoding_,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleChangeEncodingTriggered (QAction*)));

		RecentlyClosed_ = moreMenu->addMenu (tr ("Recently closed"));
		RecentlyClosed_->setEnabled (false);
		RecentlyClosedAction_ = RecentlyClosed_->menuAction ();
		RecentlyClosedAction_->setShortcutContext (Qt::WindowShortcut);

		ExternalLinks_ = new QMenu (this);
		ExternalLinks_->menuAction ()->setText (tr ("External links"));
		ExternalLinks_->menuAction ()->
			setProperty ("ActionIcon", "poshuku_externalentities");

		ExternalLinksAction_ = new QAction (this);
		ExternalLinksAction_->setText ("External links");
		ExternalLinksAction_->setProperty ("ActionIcon", "poshuku_rss");

		QWidgetAction *addressBar = new QWidgetAction (this);
		addressBar->setDefaultWidget (Ui_.URLFrame_);
		ToolBar_->addAction (addressBar);

		static_cast<QVBoxLayout*> (layout ())->insertWidget (0, ToolBar_);

		connect (ReloadPeriodically_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleReloadPeriodically ()));
		connect (NotifyWhenFinished_,
				SIGNAL (triggered ()),
				this,
				SIGNAL (invalidateSettings ()));
		connect (WebView_,
				SIGNAL (addToFavorites (const QString&, const QString&)),
				this,
				SIGNAL (addToFavorites (const QString&, const QString&)));
		connect (Add2Favorites_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAdd2Favorites ()));
		connect (Print_,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePrinting ()));
		connect (PrintPreview_,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePrintingWithPreview ()));
		connect (Find_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFind ()));
		connect (ScreenSave_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleScreenSave ()));
		connect (ViewSources_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleViewSources ()));
		connect (SavePage_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSavePage ()));
		connect (ZoomIn_,
				SIGNAL (triggered ()),
				WebView_,
				SLOT (zoomIn ()));
		connect (ZoomOut_,
				SIGNAL (triggered ()),
				WebView_,
				SLOT (zoomOut ()));
		connect (ZoomReset_,
				SIGNAL (triggered ()),
				WebView_,
				SLOT (zoomReset ()));

		connect (Ui_.URLFrame_,
				SIGNAL (load (const QString&)),
				this,
				SLOT (handleURLFrameLoad (const QString&)));

		connect (WebView_,
				SIGNAL (titleChanged (const QString&)),
				this,
				SIGNAL (titleChanged (const QString&)));
		connect (WebView_,
				SIGNAL (titleChanged (const QString&)),
				this,
				SLOT (updateLogicalPath ()));
		connect (WebView_,
				SIGNAL (urlChanged (const QString&)),
				this,
				SLOT (handleUrlChanged (const QString&)));
		connect (WebView_,
				SIGNAL (urlChanged (const QString&)),
				this,
				SLOT (updateLogicalPath ()));
		connect (WebView_,
				SIGNAL (loadProgress (int)),
				this,
				SLOT (handleLoadProgress (int)));
		connect (WebView_,
				SIGNAL (loadProgress (int)),
				this,
				SLOT (handleIconChanged ()));
		connect (WebView_,
				SIGNAL (iconChanged ()),
				this,
				SLOT (handleIconChanged ()));
		connect (WebView_,
				SIGNAL (statusBarMessage (const QString&)),
				this,
				SLOT (handleStatusBarMessage (const QString&)));
		connect (WebView_,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (WebView_,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
		connect (WebView_,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
				this,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
		connect (WebView_->page (),
				SIGNAL (linkHovered (const QString&,
						const QString&,
						const QString&)),
				this,
				SLOT (handleStatusBarMessage (const QString&)));
		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (checkLinkRels ()));
		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (setScrollPosition ()));
		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (pageFocus ()));
		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (updateTooltip ()));
		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (notifyLoadFinished (bool)));
		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleIconChanged ()));
		connect (WebView_,
				SIGNAL (loadStarted ()),
				this,
				SLOT (enableActions ()));
		connect (WebView_,
				SIGNAL (printRequested (QWebFrame*)),
				this,
				SLOT (handleViewPrint (QWebFrame*)));
		connect (WebView_,
				SIGNAL (closeRequested ()),
				this,
				SIGNAL (needToClose ()));
		connect (WebView_->page (),
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
				this,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));

		connect (&Core::Instance (),
				SIGNAL (newUnclose (QAction*)),
				this,
				SLOT (handleNewUnclose (QAction*)));

		connect (HistoryAction_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleShortcutHistory ()));

		connect (BookmarksAction_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleShortcutBookmarks ()));

		QTimer::singleShot (100,
				this,
				SLOT (focusLineEdit ()));

		connect (WebView_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (updateBookmarksState (bool)));
		FindDialog_ = new FindDialog (Ui_.WebFrame_);
		FindDialog_->hide ();

		connect (FindDialog_,
				SIGNAL (next (const QString&, QWebPage::FindFlags)),
				this,
				SLOT (findText (const QString&, QWebPage::FindFlags)));

		RememberDialog_ = new PasswordRemember (Ui_.WebFrame_);
		RememberDialog_->hide ();

		connect (WebView_,
				SIGNAL (storeFormData (const PageFormsData_t&)),
				RememberDialog_,
				SLOT (add (const PageFormsData_t&)));

		updateLogicalPath ();
	}

	BrowserWidget::~BrowserWidget ()
	{
		if (Own_)
			Core::Instance ().Unregister (this);
	}

	void BrowserWidget::SetParentMultiTabs (QObject *parent)
	{
		S_MultiTabsParent_ = parent;
	}

	void BrowserWidget::Deown ()
	{
		Own_ = false;
	}

	void BrowserWidget::InitShortcuts ()
	{
		IShortcutProxy *proxy = Core::Instance ().GetShortcutProxy ();
		QObject *object = Core::Instance ().parent ();

		Cut_->setShortcuts (proxy->GetShortcuts (object, "BrowserCut_"));
		Copy_->setShortcuts (proxy->GetShortcuts (object, "BrowserCopy_"));
		Paste_->setShortcuts (proxy->GetShortcuts (object, "BrowserPaste_"));
		Back_->setShortcuts (proxy->GetShortcuts (object, "BrowserBack_"));
		Forward_->setShortcuts (proxy->GetShortcuts (object, "BrowserForward_"));
		Reload_->setShortcuts (proxy->GetShortcuts (object, "BrowserReload_"));
		Stop_->setShortcuts (proxy->GetShortcuts (object, "BrowserStop_"));
		Add2Favorites_->setShortcuts (proxy->GetShortcuts (object, "BrowserAdd2Favorites_"));
		Find_->setShortcuts (proxy->GetShortcuts (object, "BrowserFind_"));
		Print_->setShortcuts (proxy->GetShortcuts (object, "BrowserPrint_"));
		PrintPreview_->setShortcuts (proxy->GetShortcuts (object, "BrowserPrintPreview_"));
		ScreenSave_->setShortcuts (proxy->GetShortcuts (object, "BrowserScreenSave_"));
		ViewSources_->setShortcuts (proxy->GetShortcuts (object, "BrowserViewSources_"));
		ZoomIn_->setShortcuts (proxy->GetShortcuts (object, "BrowserZoomIn_"));
		ZoomOut_->setShortcuts (proxy->GetShortcuts (object, "BrowserZoomOut_"));
		ZoomReset_->setShortcuts (proxy->GetShortcuts (object, "BrowserZoomReset_"));
		RecentlyClosedAction_->setShortcuts (proxy->GetShortcuts (object, "BrowserRecentlyClosedAction_"));
	}

	void BrowserWidget::SetUnclosers (const QList<QAction*>& unclosers)
	{
		RecentlyClosed_->addActions (unclosers);
		if (unclosers.size ())
		{
			RecentlyClosed_->setEnabled (true);
			RecentlyClosed_->setDefaultAction (unclosers.front ());
			connect (RecentlyClosedAction_,
					SIGNAL (triggered ()),
					unclosers.front (),
					SLOT (trigger ()));

			foreach (QAction *action, unclosers)
			{
				connect (action,
						SIGNAL (destroyed (QObject*)),
						this,
						SLOT (handleUncloseDestroyed ()));
			}
		}
	}

	QGraphicsView* BrowserWidget::GetGraphicsView () const
	{
		return Ui_.WebGraphicsView_;
	}

	CustomWebView* BrowserWidget::GetView () const
	{
		return WebView_;
	}

	QLineEdit* BrowserWidget::GetURLEdit () const
	{
		return Ui_.URLFrame_->GetEdit ();
	}

	BrowserWidgetSettings BrowserWidget::GetWidgetSettings () const
	{
		QByteArray ba;
		QDataStream out (&ba, QIODevice::WriteOnly);
		out << *WebView_->page ()->history ();
		BrowserWidgetSettings result =
		{
			WebView_->zoomFactor (),
			NotifyWhenFinished_->isChecked (),
			QTime (0, 0, 0).addMSecs (ReloadTimer_->interval ()),
			ba,
			WebView_->page ()->mainFrame ()->scrollPosition ()
		};
		return result;
	}

	void BrowserWidget::SetWidgetSettings (const BrowserWidgetSettings& settings)
	{
		if (settings.ZoomFactor_ != 1)
		{
			qDebug () << Q_FUNC_INFO
				<< "setting zoomfactor to"
				<< settings.ZoomFactor_;
			WebView_->setZoomFactor (settings.ZoomFactor_);
		}

		NotifyWhenFinished_->setChecked (settings.NotifyWhenFinished_);
		QTime interval = settings.ReloadInterval_;
		QTime null (0, 0, 0);
		int msecs = null.msecsTo (interval);
		if (msecs >= 1000)
		{
			ReloadPeriodically_->setChecked (true);
			SetActualReloadInterval (interval);
		}

		if (settings.WebHistorySerialized_.size ())
		{
			QDataStream str (settings.WebHistorySerialized_);
			str >> *WebView_->page ()->history ();
		}

		if (!settings.ScrollPosition_.isNull ())
			SetOnLoadScrollPoint (settings.ScrollPosition_);
	}

	void BrowserWidget::SetURL (const QUrl& thurl)
	{
		QUrl url = thurl;
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookSetURL (proxy, this, url);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("url", url);

		if (!url.isEmpty () && url.isValid ())
		{
			HtmlMode_ = false;
			WebView_->Load (url);
		}
	}

	void BrowserWidget::Load (const QString& url)
	{
		SetURL (Core::Instance ().MakeURL (url));
	}

	void BrowserWidget::SetHtml (const QString& html, const QUrl& base)
	{
		Ui_.URLFrame_->GetEdit ()->clear ();
		HtmlMode_ = true;
		WebView_->setHtml (html, base);
	}

	void BrowserWidget::SetNavBarVisible (bool visible)
	{
		ToolBar_->setVisible (visible);
	}

	void BrowserWidget::SetEverythingElseVisible (bool visible)
	{
		if (!visible)
			Ui_.Sidebar_->hide ();
		Ui_.Splitter_->handle (1)->setVisible (visible);
	}

	QWidget* BrowserWidget::Widget ()
	{
		return this;
	}

#define _LC_MERGE(a) "Browser"#a

#define _LC_SINGLE(a) \
		name2act [_LC_MERGE(a)] = a;

#define _LC_TRAVERSER(z,i,array) \
		_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))

#define _LC_EXPANDER(Names) \
		BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names)
	void BrowserWidget::SetShortcut (const QString& name, const QKeySequences_t& sequences)
	{
		QMap<QString, QAction*> name2act;
		_LC_EXPANDER ((Add2Favorites_)
				(Find_)
				(Print_)
				(PrintPreview_)
				(ScreenSave_)
				(ViewSources_)
				(ZoomIn_)
				(ZoomOut_)
				(ZoomReset_)
				(Cut_)
				(Copy_)
				(Paste_)
				(Back_)
				(Forward_)
				(Reload_)
				(Stop_)
				(RecentlyClosedAction_));
		if (name2act.contains (name))
			name2act [name]->setShortcuts (sequences);
	}

#define _L(a,b) result ["Browser"#a] = ActionInfo (a->text (), \
			b, a->icon ())
	QMap<QString, ActionInfo> BrowserWidget::GetActionInfo () const
	{
		QMap<QString, ActionInfo> result;
		_L (Add2Favorites_, tr ("Ctrl+D"));
		_L (Find_, tr ("Ctrl+F"));
		_L (Print_, tr ("Ctrl+P"));
		_L (PrintPreview_, tr ("Ctrl+Shift+P"));
		_L (ScreenSave_, Qt::Key_F12);
		_L (ViewSources_, tr ("Ctrl+Shift+V"));
		_L (ZoomIn_, Qt::CTRL + Qt::Key_Plus);
		_L (ZoomOut_, Qt::CTRL + Qt::Key_Minus);
		_L (ZoomReset_, tr ("Ctrl+0"));
		_L (Cut_, tr ("Ctrl+X"));
		_L (Copy_, tr ("Ctrl+C"));
		_L (Paste_, tr ("Ctrl+V"));
		_L (Back_, Qt::ALT + Qt::Key_Left);
		_L (Forward_, Qt::ALT + Qt::Key_Right);
		_L (Reload_, Qt::Key_F5);
		_L (Stop_, Qt::Key_Escape);
		_L (RecentlyClosedAction_, tr ("Ctrl+Shift+T"));
		return result;
	}

	void BrowserWidget::Remove ()
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookTabRemoveRequested (proxy, this);
		if (proxy->IsCancelled ())
			return;

		emit needToClose ();
	}

	QToolBar* BrowserWidget::GetToolBar () const
	{
		return Own_ ? ToolBar_ : 0;
	}

	namespace
	{
		void Append (QList<QAction*>& result, const QList<QObject*>& objs)
		{
			Q_FOREACH (QObject *obj, objs)
			{
				QAction *act = qobject_cast<QAction*> (obj);
				if (!act)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< obj
							<< "from plugins to QAction*";
					continue;
				}
				result << act;
			}
		}
	}

	QList<QAction*> BrowserWidget::GetTabBarContextMenuActions () const
	{
		QList<QObject*> plugResult;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookTabBarContextMenuActions (proxy, this);
		proxy->FillValue ("actions", plugResult);

		QList<QAction*> result;
		Append (result, plugResult);

		if (!proxy->IsCancelled ())
			result << Reload_
				<< NotifyWhenFinished_
				<< Add2Favorites_
				<< RecentlyClosedAction_
				<< Print_
				<< Back_;

		plugResult.clear ();
		proxy->FillValue ("endActions", plugResult);
		Append (result, plugResult);

		return result;
	}

	QMap<QString, QList<QAction*> > BrowserWidget::GetWindowMenus () const
	{
		return WindowMenus_;
	}

	QObject* BrowserWidget::ParentMultiTabs ()
	{
		return S_MultiTabsParent_;
	}

	TabClassInfo BrowserWidget::GetTabClassInfo () const
	{
		return Core::Instance ().GetTabClass ();
	}

	void BrowserWidget::SetOnLoadScrollPoint (const QPoint& sp)
	{
		OnLoadPos_ = sp;
	}

	void BrowserWidget::PrintImpl (bool preview, QWebFrame *frame)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookPrint (proxy, this, preview, frame);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("preview", preview);

		std::auto_ptr<QPrinter> printer (new QPrinter ());

		QPrintDialog *dialog = new QPrintDialog (printer.get (), this);
		dialog->setWindowTitle (tr ("Print web page"));
		/* TODO
		if (!WebView_->selectedText ().isEmpty ())
			dialog->addEnabledOption (QAbstractPrintDialog::PrintSelection);
		*/

		if (dialog->exec () != QDialog::Accepted)
			return;

		if (preview)
		{
			QPrintPreviewDialog *prevDialog =
				new QPrintPreviewDialog (printer.get (), this);
			connect (prevDialog,
					SIGNAL (paintRequested (QPrinter*)),
					frame,
					SLOT (print (QPrinter*)));

			if (prevDialog->exec () != QDialog::Accepted)
				return;
		}

		frame->print (printer.get ());
	}

	void BrowserWidget::SetActualReloadInterval (const QTime& value)
	{
		QTime null (0, 0, 0);
		int msecs = null.msecsTo (value);
		QString tip = tr ("Reloading once in %1")
			.arg (value.toString ());
		ReloadPeriodically_->setStatusTip (tip);
		ReloadPeriodically_->setToolTip (tip);
		ReloadTimer_->start (msecs);
	}

	void BrowserWidget::handleIconChanged ()
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookIconChanged (proxy, WebView_->page (), this);
		if (proxy->IsCancelled ())
			return;

		QIcon icon = WebView_->icon ();
		if (icon.isNull ())
			icon = Core::Instance ().GetIcon (WebView_->url ());

		Ui_.URLFrame_->SetFavicon (icon);

		emit iconChanged (icon);
	}

	void BrowserWidget::handleStatusBarMessage (const QString& thmsg)
	{
		QString msg = thmsg;
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookStatusBarMessage (proxy, this, msg);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("message", msg);

		if (msg.isEmpty ())
		{
			LinkTextItem_.reset ();
			return;
		}

		LinkTextItem_.reset (new QGraphicsTextItem (WebView_));
		LinkTextItem_->setZValue (1);

		const QFontMetrics metrics (LinkTextItem_->font ());

		msg = metrics.elidedText (msg, Qt::ElideMiddle, WebView_->boundingRect ().width () * 2 / 3);

		LinkTextItem_->setPlainText (msg);

		const int textHeight = metrics.boundingRect (msg).height ();
		const qreal x = 1;
		const qreal y = WebView_->boundingRect ().height () - textHeight - 7;
		LinkTextItem_->setX (x);
		LinkTextItem_->setY (y);

		QGraphicsRectItem *rect = new QGraphicsRectItem (0, 0,
				LinkTextItem_->boundingRect ().width (),
				LinkTextItem_->boundingRect ().height (),
				LinkTextItem_.get ());
		rect->setFlag (QGraphicsItem::ItemStacksBehindParent);
		rect->setBrush (palette ().color (QPalette::AlternateBase));
		rect->setPen (QPen (palette ().color (QPalette::Text), 0.5));

		QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect ();
		eff->setOpacity (0.8);
		rect->setGraphicsEffect (eff);
	}

	void BrowserWidget::handleURLFrameLoad (const QString& text)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookURLEditReturnPressed (proxy, this);
		if (proxy->IsCancelled ())
			return;

		Load (text);
	}

	void BrowserWidget::handleReloadPeriodically ()
	{
		if (ReloadPeriodically_->isChecked ())
		{
			std::auto_ptr<ReloadIntervalSelector> sel (new ReloadIntervalSelector (this));
			if (sel->exec () != QDialog::Accepted)
			{
				ReloadPeriodically_->setChecked (false);
				ReloadPeriodically_->setStatusTip (QString ());
				ReloadPeriodically_->setToolTip (QString ());
				ReloadTimer_->stop ();
				return;
			}

			QTime value = sel->GetInterval ();
			QTime null (0, 0, 0);
			int msecs = null.msecsTo (value);
			if (msecs < 1000)
			{
				ReloadPeriodically_->setChecked (false);
				ReloadPeriodically_->setStatusTip (QString ());
				ReloadPeriodically_->setToolTip (QString ());
				ReloadTimer_->stop ();
				return;
			}

			SetActualReloadInterval (value);
		}
		else if (ReloadTimer_->isActive ())
		{
			ReloadPeriodically_->setStatusTip (QString ());
			ReloadPeriodically_->setToolTip (QString ());
			ReloadTimer_->stop ();
		}

		emit invalidateSettings ();
	}

	void BrowserWidget::handleAdd2Favorites ()
	{
		const QString& url = WebView_->url ().toString ();
		checkPageAsFavorite (url);

		if (Core::Instance ().IsUrlInFavourites (url))
			Core::Instance ().GetFavoritesModel ()->removeItem (url);
		else
			emit addToFavorites (WebView_->title (), url);
	}

	void BrowserWidget::handleFind ()
	{
		QAction *act = qobject_cast<QAction*> (sender ());
		if (act)
			FindDialog_->SetText (act->data ().toString ());
		FindDialog_->show ();
		FindDialog_->Focus ();
	}

	void BrowserWidget::findText (const QString& thtext,
			QWebPage::FindFlags flags)
	{
		QString text = thtext;
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFindText (proxy, this, text, flags);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("text", text);

		if (PreviousFindText_ != text)
		{
			QWebPage::FindFlags nflags = flags | QWebPage::HighlightAllOccurrences;
			WebView_->page ()->findText (text, nflags);
			PreviousFindText_ = text;
		}
		bool found = WebView_->page ()->findText (text, flags);
		FindDialog_->SetSuccessful (found);
	}

	void BrowserWidget::handleViewPrint (QWebFrame *frame)
	{
		PrintImpl (false, frame);
	}

	void BrowserWidget::handlePrinting ()
	{
		PrintImpl (false, WebView_->page ()->mainFrame ());
	}

	void BrowserWidget::handlePrintingWithPreview ()
	{
		PrintImpl (true, WebView_->page ()->mainFrame ());
	}

	void BrowserWidget::handleScreenSave ()
	{
		QSize contentsSize = WebView_->page ()->mainFrame ()->contentsSize ();
		QSize oldSize = WebView_->page ()->viewportSize ();
		QRegion clip (0, 0, contentsSize.width (), contentsSize.height ());

		QPixmap image (contentsSize);
		QPainter painter (&image);
		WebView_->page ()->setViewportSize (contentsSize);
		WebView_->page ()->mainFrame ()->render (&painter, clip);
		WebView_->page ()->setViewportSize (oldSize);

		std::auto_ptr<ScreenShotSaveDialog> dia (new ScreenShotSaveDialog (image, this));
		if (dia->exec () != QDialog::Accepted)
			return;

		QString filename = QFileDialog::getSaveFileName (this,
				tr ("Save screenshot"),
				XmlSettingsManager::Instance ()->
					Property ("ScreenshotsLocation",
						QDesktopServices::storageLocation (
							QDesktopServices::DocumentsLocation)).toString ());
		if (filename.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("ScreenshotsLocation", filename);

		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not open %1 for write")
						.arg (filename));
			return;
		}

		if (!file.write (dia->Save ()))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not write screenshot to %1")
						.arg (filename));
			return;
		}
	}

	void BrowserWidget::handleViewSources ()
	{
		QString html = WebView_->page ()->mainFrame ()->toHtml ();

		Entity e = Util::MakeEntity (html,
				QString (),
				FromUserInitiated,
				"x-leechcraft/plain-text-document");
		e.Additional_ ["Language"] = "HTML";
		e.Additional_ ["IsTemporaryDocument"] = true;
		bool ch = false;
		emit couldHandle (e, &ch);
		if (ch)
		{
			emit gotEntity (e);
			return;
		}

		SourceViewer *viewer = new SourceViewer (this);
		viewer->setAttribute (Qt::WA_DeleteOnClose);
		viewer->SetHtml (html);
		viewer->show ();
	}

	void BrowserWidget::handleSavePage ()
	{
		Entity e = Util::MakeEntity (WebView_->url (),
				QString (),
				FromUserInitiated);
		e.Additional_ ["AllowedSemantics"] = QStringList ("fetch") << "save";
		emit gotEntity (e);
	}

	void BrowserWidget::focusLineEdit ()
	{
		QLineEdit *edit = Ui_.URLFrame_->GetEdit ();
		edit->setFocus (Qt::OtherFocusReason);
		edit->selectAll ();
	}

	void BrowserWidget::updateBookmarksState (bool)
	{
		checkPageAsFavorite (WebView_->url ().toString ());
	}

	QGraphicsWebView* BrowserWidget::getWebView () const
	{
		return WebView_;
	}

	QLineEdit* BrowserWidget::getAddressBar () const
	{
		return Ui_.URLFrame_->GetEdit ();
	}

	QWidget* BrowserWidget::getSideBar () const
	{
		return Ui_.Sidebar_;
	}

	void BrowserWidget::handleNewUnclose (QAction *action)
	{
		QList<QAction*> actions = RecentlyClosed_->actions ();
		if (actions.size ())
			RecentlyClosed_->insertAction (actions.first (), action);
		else
		{
			RecentlyClosed_->addAction (action);
		}
		RecentlyClosed_->setEnabled (true);
		RecentlyClosed_->setDefaultAction (action);
		disconnect (RecentlyClosedAction_,
				SIGNAL (triggered ()),
				0,
				0);
		connect (RecentlyClosedAction_,
				SIGNAL (triggered ()),
				action,
				SLOT (trigger ()));
		connect (action,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleUncloseDestroyed ()));
	}

	void BrowserWidget::handleUncloseDestroyed ()
	{
		if (!RecentlyClosed_->actions ().size ())
			RecentlyClosed_->setEnabled (false);
		else
		{
			disconnect (RecentlyClosedAction_,
					SIGNAL (triggered ()),
					0,
					0);
			connect (RecentlyClosedAction_,
					SIGNAL (triggered ()),
					RecentlyClosed_->actions ().front (),
					SLOT (trigger ()));
			RecentlyClosed_->setDefaultAction (RecentlyClosed_->actions ().front ());
		}
	}

	void BrowserWidget::updateTooltip ()
	{
		if (!XmlSettingsManager::Instance ()->
				property ("GenerateTooltips").toBool ())
			return;

		const int previewWidth = 400;
		if (!WebView_->size ().isValid ())
			return;

		QSize contentsSize = WebView_->page ()->mainFrame ()->contentsSize ();
		if (contentsSize.width () < 800)
			contentsSize.scale (800, 1, Qt::KeepAspectRatioByExpanding);
		int maxHeight = 0.8 * QApplication::desktop ()->
			screenGeometry (this).height () * static_cast<double> (contentsSize.width ()) / previewWidth;
		contentsSize.setHeight (std::min (contentsSize.height (), 3000));
		QPoint scroll = WebView_->page ()->mainFrame ()->scrollPosition ();
		QSize oldSize = WebView_->page ()->viewportSize ();
		QRegion clip (0, 0, contentsSize.width (), contentsSize.height ());

		QPixmap pixmap (contentsSize);
		if (pixmap.isNull ())
			return;

		pixmap.fill (QColor (0, 0, 0, 0));

		QPainter painter (&pixmap);
		WebView_->page ()->setViewportSize (contentsSize);
		WebView_->page ()->mainFrame ()->render (&painter, clip);
		WebView_->page ()->setViewportSize (oldSize);
		WebView_->page ()->mainFrame ()->setScrollPosition (scroll);
		painter.end ();

		QLabel *widget = new QLabel;

		if (pixmap.height () > 3000)
			pixmap = pixmap.copy (0, 0, pixmap.width (), 3000);

		pixmap = pixmap.scaledToWidth (previewWidth, Qt::SmoothTransformation);
		maxHeight = 0.8 * QApplication::desktop ()->screenGeometry (this).height ();
		if (pixmap.height () > maxHeight)
			pixmap = pixmap.copy (0, 0, previewWidth, maxHeight);
		widget->setPixmap (pixmap);
		widget->setFixedSize (pixmap.width (), pixmap.height ());

		emit tooltipChanged (widget);
	}

	void BrowserWidget::enableActions ()
	{
		Add2Favorites_->setEnabled (true);
		Find_->setEnabled (true);
		Print_->setEnabled (true);
		PrintPreview_->setEnabled (true);
		ScreenSave_->setEnabled (true);
		ViewSources_->setEnabled (true);
		SavePage_->setEnabled (true);
	}

	void BrowserWidget::handleEntityAction ()
	{
		emit gotEntity (qobject_cast<QAction*> (sender ())->
				data ().value<LeechCraft::Entity> ());
	}

	void BrowserWidget::checkLinkRels ()
	{
		if (HtmlMode_)
			return;

		IAddressBar *iab = qobject_cast<IAddressBar*> (GetURLEdit ());
		if (!iab)
		{
			qWarning () << Q_FUNC_INFO
				<< GetURLEdit ()
				<< "isn't an IAddressBar object";
			return;
		}

		iab->RemoveAction (ExternalLinksAction_);

		ExternalLinks_->clear ();

		QWebElementCollection links = WebView_->
				page ()->mainFrame ()->findAllElements ("link");
		QUrl mainFrameURL = WebView_->page ()->mainFrame ()->url ();
		bool inserted = false;
		Q_FOREACH (QWebElement link, links)
		{
			if (link.attribute ("type") == "")
				continue;

			LeechCraft::Entity e;
			e.Mime_ = link.attribute ("type");

			QString entity = link.attribute ("title");
			if (entity.isEmpty ())
			{
				entity = e.Mime_;
				entity.remove ("application/");
				entity.remove ("+xml");
				entity = entity.toUpper ();
			}

			QUrl entityUrl = mainFrameURL.resolved (QUrl (link.attribute ("href")));
			e.Entity_ = entityUrl;
			e.Additional_ ["SourceURL"] = entityUrl;
			e.Parameters_ = LeechCraft::FromUserInitiated |
				LeechCraft::OnlyHandle;
			e.Additional_ ["UserVisibleName"] = entity;
			e.Additional_ ["LinkRel"] = link.attribute ("rel");

			bool ch = false;
			emit couldHandle (e, &ch);
			if (ch)
			{
				QString mime = e.Mime_;
				mime.replace ('/', '_');
				QAction *act = ExternalLinks_->
					addAction (QIcon (QString (":/resources/images/%1.png")
							.arg (mime)),
						entity,
						this,
						SLOT (handleEntityAction ()));
				act->setData (QVariant::fromValue<LeechCraft::Entity> (e));
				if (!inserted)
				{
					QToolButton *btn = iab->InsertAction (ExternalLinksAction_);
					iab->SetVisible (ExternalLinksAction_, true);
					btn->setMenu (ExternalLinks_);
					btn->setArrowType (Qt::NoArrow);
					btn->setPopupMode (QToolButton::InstantPopup);
					const QString newStyle ("::menu-indicator { image: "
							"url(data:image/gif;base64,R0lGODlhAQABAPABAP///"
							"wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw==);}");
					btn->setStyleSheet (btn->styleSheet () + newStyle);

					connect (ExternalLinks_->menuAction (),
							SIGNAL (triggered ()),
							this,
							SLOT (showSendersMenu ()),
							Qt::UniqueConnection);
					inserted = true;
				}
			}
		}
	}

	void BrowserWidget::setScrollPosition ()
	{
		if (!OnLoadPos_.isNull ())
		{
			GetView ()->page ()->mainFrame ()->setScrollPosition (OnLoadPos_);
			OnLoadPos_ = QPoint ();
		}
	}

	void BrowserWidget::pageFocus ()
	{
		if (!HtmlMode_ && isVisible ())
			WebView_->setFocus ();
	}

	void BrowserWidget::handleLoadProgress (int p)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookLoadProgress (proxy, WebView_->page (), this, p);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("progress", p);

		QString title = WebView_->title ();
		if (p > 0 && p < 100)
			title.prepend (QString ("[%1%] ").arg (p));
		emit titleChanged (title);

		QAction *o = 0;
		QAction *n = 0;
		QString actionIcon = "poshuku_";
		if (p < 100 && p > 0)
		{
			o = Reload_;
			n = Stop_;
			actionIcon += "stop";
		}
		else
		{
			o = Stop_;
			n = Reload_;
			actionIcon += "reload";
		}
		disconnect (ReloadStop_,
				SIGNAL (triggered ()),
				o,
				SLOT (trigger ()));
		ReloadStop_->setIcon (n->icon ());
		ReloadStop_->setShortcut (n->shortcut ());
		ReloadStop_->setText (n->text ());
		ReloadStop_->setProperty ("ActionIcon", actionIcon);
		connect (ReloadStop_,
				SIGNAL (triggered ()),
				n,
				SLOT (trigger ()));
	}

	void BrowserWidget::notifyLoadFinished (bool ok)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookNotifyLoadFinished (proxy,
				WebView_,
				this,
				ok,
				NotifyWhenFinished_->isChecked (),
				Own_,
				HtmlMode_);

		proxy->FillValue ("ok", ok);

		if (!NotifyWhenFinished_->isChecked () ||
				!Own_ ||
				HtmlMode_ ||
				isVisible ())
			return;

		QString h = WebView_->title ();
		if (h.isEmpty ())
			h = WebView_->url ().toString ();
		if (h.isEmpty ())
			return;

		QString text;
		Priority prio = PInfo_;

		if (ok)
			text = tr ("Page load finished: %1")
				.arg (Qt::escape (WebView_->title ()));
		else
		{
			text = tr ("Page load failed: %1")
				.arg (Qt::escape (WebView_->title ()));
			prio = PWarning_;
		}

		Entity e = Util::MakeNotification ("Poshuku", text, prio);
		Util::NotificationActionHandler *nh = new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open"), boost::bind (&BrowserWidget::raiseTab, this, this));
		nh->AddDependentObject (this);
		emit gotEntity (e);
	}

	void BrowserWidget::handleChangeEncodingAboutToShow ()
	{
		ChangeEncoding_->clear ();

		QStringList codecs;
		QList<int> mibs = QTextCodec::availableMibs ();
		QMap<QString, int> name2mib;
		Q_FOREACH (int mib, mibs)
		{
			QString name = QTextCodec::codecForMib (mib)->name ();
			codecs << name;
			name2mib [name] = mib;
		}
		codecs.sort ();

		QString defaultEncoding = WebView_->
			settings ()->defaultTextEncoding ();
		const int currentCodec = codecs.indexOf (defaultEncoding);

		QAction *def = ChangeEncoding_->addAction (tr ("Default"));
		def->setData (-1);
		def->setCheckable (true);
		if (currentCodec == -1)
			def->setChecked (true);
		ChangeEncoding_->addSeparator ();

		for (int i = 0; i < codecs.count (); ++i)
		{
			QAction *cdc = ChangeEncoding_->addAction (codecs.at (i));
			cdc->setData (name2mib [codecs.at (i)]);
			cdc->setCheckable (true);
			if (currentCodec == i)
				cdc->setChecked (true);
		}
	}

	void BrowserWidget::handleChangeEncodingTriggered (QAction *action)
	{
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
				<< "action is null";
			return;
		}

		int mib = action->data ().toInt ();
		QString encoding;
		if (mib >= 0)
			encoding = QTextCodec::codecForMib (mib)->name ();
		WebView_->settings ()->setDefaultTextEncoding (encoding);
		Reload_->trigger ();
	}

	void BrowserWidget::updateLogicalPath ()
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookUpdateLogicalPath (proxy, this);
		if (proxy->IsCancelled ())
			return;

		static QStringList skip;

		skip << "org.ru"
			<< "net.ru";

		QUrl url = WebView_->url ();
		QString title = WebView_->title ();
		if (title.isEmpty ())
			title = tr ("No title");
		QString host = url.host ();
		host.remove ("www.");
		QStringList path;
		path << (host.isEmpty () ? QString ("Poshuku") : host);
		path << title;

		QStringList domains = host.split ('.', QString::SkipEmptyParts);
		while (domains.size () > 2)
		{
			domains.takeFirst ();
			QString joined = domains.join (".");
			if (skip.contains (joined))
				continue;
			path.prepend (joined);
		}

		setProperty ("WidgetLogicalPath", path);
	}

	void BrowserWidget::showSendersMenu ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QAction"
				<< sender ();
			return;
		}

		QMenu *menu = action->menu ();
		menu->exec (QCursor::pos ());
	}

	void BrowserWidget::handleUrlChanged (const QString& value)
	{
		QString userText = value;
#ifdef ENABLE_IDN
		if (userText.contains ("xn--"))
		{
			QRegExp rx ("(?://|\\.)xn--(.+)(?:\\.|/)");
			rx.setMinimal (true);
			int pos = 0;
			QStringList caps;

			while ((pos = rx.indexIn (userText, pos)) != -1)
			{
				caps << rx.cap (1);
				pos += rx.matchedLength () - 4;
			}

			Q_FOREACH (QString str, caps)
			{
				str.prepend ("xn--");
				char *output = 0;
				idna_to_unicode_8z8z (str.toUtf8 ().constData (),
						&output, IDNA_ALLOW_UNASSIGNED);
				QString newStr = QString::fromUtf8 (output);
				userText.replace (str, newStr);
			}
		}
#endif
		Ui_.URLFrame_->GetEdit ()->setText (userText);
		Ui_.URLFrame_->GetEdit ()->repaint ();

		emit urlChanged (value);
	}

	void BrowserWidget::refitWebView ()
	{
		WebView_->resize (Ui_.WebGraphicsView_->viewport ()->size ());
		Ui_.WebGraphicsView_->ensureVisible (WebView_, 0, 0);
		Ui_.WebGraphicsView_->centerOn (WebView_);
	}

	void BrowserWidget::handleUrlTextChanged (const QString& url)
	{
		IAddressBar *iab = qobject_cast<IAddressBar*> (GetURLEdit ());
		if (!iab)
		{
			qWarning () << Q_FUNC_INFO
					<< GetURLEdit ()
					<< "isn't an IAddressBar object";
			return;
		}
		checkPageAsFavorite (url);
	}

	void BrowserWidget::checkPageAsFavorite (const QString& url)
	{
		if (url != WebView_->url ().toString () &&
				url != GetURLEdit ()->text ())
			return;

		if (Core::Instance ().IsUrlInFavourites (url))
		{
			Add2Favorites_->setProperty ("ActionIcon", "poshuku_removebookmark");
			Add2Favorites_->setText (tr ("Remove bookmark"));
			Add2Favorites_->setToolTip (tr ("Remove bookmark"));

			IAddressBar *iab = qobject_cast<IAddressBar*> (GetURLEdit ());
			if (!iab)
				qWarning () << Q_FUNC_INFO
						<< GetURLEdit ()
						<< "isn't an IAddressBar object";
			else
			{
				QToolButton *btn = iab->GetButtonFromAction (Add2Favorites_);
				if (btn)
					btn->setIcon (Core::Instance ().GetProxy ()->GetIcon ("poshuku_removebookmark"));
			}
		}
		else
		{
			Add2Favorites_->setProperty ("ActionIcon", "poshuku_addbookmark");
			Add2Favorites_->setText (tr ("Add bookmark"));
			Add2Favorites_->setToolTip (tr ("Add bookmark"));

			IAddressBar *iab = qobject_cast<IAddressBar*> (GetURLEdit ());
			if (!iab)
				qWarning () << Q_FUNC_INFO
						<< GetURLEdit ()
						<< "isn't an IAddressBar object";
			else
			{
				QToolButton *btn = iab->GetButtonFromAction (Add2Favorites_);
				if (btn)
					btn->setIcon (Core::Instance ().GetProxy ()->GetIcon ("poshuku_addbookmark"));
			}
		}
	}

	void BrowserWidget::handleShortcutHistory ()
	{
		if (!HistoryAction_->isChecked ())
			HistoryAction_->setChecked (false);
		else
		{
			HistoryAction_->setChecked (true);
			BookmarksAction_->setChecked (false);
		}

		SetSplitterSizes (1);
	}

	void BrowserWidget::handleShortcutBookmarks ()
	{
		if (!BookmarksAction_->isChecked ())
			BookmarksAction_->setChecked (false);
		else
		{
			HistoryAction_->setChecked (false);
			BookmarksAction_->setChecked (true);
		}

		SetSplitterSizes (0);
	}

	void BrowserWidget::SetSplitterSizes (int currentIndex)
	{
		int splitterSize = XmlSettingsManager::Instance ()->
				Property ("HistoryBoormarksPanelSize", 250).toInt ();
		int wSize = WebView_->size ().width ();

		if (!Ui_.Splitter_->sizes ().at (0))
		{
			Ui_.Splitter_->setSizes (QList<int> () << splitterSize << wSize - splitterSize);
			Ui_.Sidebar_->GetMainTabBar ()->setCurrentIndex (currentIndex);
		}
		else if (Ui_.Sidebar_->GetMainTabBar ()->currentIndex () != currentIndex)
			Ui_.Sidebar_->GetMainTabBar ()->setCurrentIndex (currentIndex);
		else
		{
			XmlSettingsManager::Instance ()->
				setProperty ("HistoryBoormarksPanelSize", Ui_.Splitter_->sizes ().at (0));
			Ui_.Splitter_->setSizes (QList<int> () <<  0 << wSize);
		}
	}
}
}
