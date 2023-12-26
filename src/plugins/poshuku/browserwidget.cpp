/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "browserwidget.h"
#include <limits>
#include <cmath>
#include <QDesktopWidget>
#include <QtDebug>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QMovie>
#include <QWidgetAction>
#include <QTimer>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QCursor>
#include <QDomDocument>
#include <QDomElement>
#include <QDataStream>
#include <QRegExp>
#include <QKeySequence>
#include <QLabel>
#include <QXmlStreamWriter>
#include <QMimeData>

#ifdef ENABLE_IDN
#include <idna.h>
#endif

#include <util/util.h>
#include <util/sll/slotclosure.h>
#include <util/sll/unreachable.h>
#include <util/xpc/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include <util/shortcuts/shortcutmanager.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ishortcutproxy.h>
#include "interfaces/poshuku/iwebview.h"
#include "interfaces/poshuku/iwebviewhistory.h"
#include "components/tabui/focustracker.h"
#include "core.h"
#include "screenshotsavedialog.h"
#include "xmlsettingsmanager.h"
#include "sourceviewer.h"
#include "passwordremember.h"
#include "reloadintervalselector.h"
#include "browserwidgetsettings.h"
#include "bookmarkswidget.h"
#include "historywidget.h"
#include "urleditbuttonsmanager.h"
#include "zoomer.h"
#include "searchtext.h"
#include "featurepermnotification.h"
#include "browserwidgetsettingshandler.h"

Q_DECLARE_METATYPE (QList<QObject*>);

namespace LC
{
namespace Poshuku
{
	using LC::ActionInfo;

	QObject *BrowserWidget::S_MultiTabsParent_ = 0;

	BrowserWidget::BrowserWidget (Kind kind, const IWebView_ptr& view,
			Util::ShortcutManager *sm, const ICoreProxy_ptr& coreProxy, QWidget *parent)
	: QWidget { parent }
	, ReloadTimer_ { new QTimer { this } }
	, WebView_ { view }
	, Proxy_ { coreProxy }
	, Kind_ { kind }
	{
		Ui_.setupUi (this);

		LinkTextItem_ = new QLabel (Ui_.WebFrame_);
		LinkTextItem_->setAutoFillBackground (true);
		LinkTextItem_->setTextFormat (Qt::TextFormat::PlainText);
		LinkTextItem_->setMargin (4);
		LinkTextItem_->raise ();
		LinkTextItem_->hide ();

		Core::Instance ().GetPluginManager ()->RegisterHookable (this);
		Ui_.Sidebar_->AddPage (tr ("Bookmarks"), new BookmarksWidget);
		Ui_.Sidebar_->AddPage (tr ("History"), new HistoryWidget);
		Ui_.Splitter_->setSizes ({ 0, 1000 });

		const auto webViewWidget = WebView_->GetQWidget ();
		Ui_.WebFrame_->layout ()->addWidget (webViewWidget);
		webViewWidget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		connect (webViewWidget,
				SIGNAL (contextMenuRequested (QPoint, ContextMenuInfo)),
				this,
				SLOT (handleContextMenu (QPoint, ContextMenuInfo)));

		connect (webViewWidget,
				SIGNAL (urlChanged (QUrl)),
				this,
				SIGNAL (tabRecoverDataChanged ()));
		connect (webViewWidget,
				SIGNAL (zoomChanged ()),
				this,
				SIGNAL (tabRecoverDataChanged ()));

		connect (ReloadTimer_,
				SIGNAL (timeout ()),
				WebView_->GetPageAction (IWebView::PageAction::Reload),
				SLOT (trigger ()));

		Cut_ = WebView_->GetPageAction (IWebView::PageAction::Cut);
		Cut_->setShortcutContext (Qt::WindowShortcut);
		Copy_ = WebView_->GetPageAction (IWebView::PageAction::Copy);
		Copy_->setShortcutContext (Qt::WindowShortcut);
		Paste_ = WebView_->GetPageAction (IWebView::PageAction::Paste);

		ToolBar_ = new QToolBar (this);
		ToolBar_->setWindowTitle ("Poshuku");

		Back_ = WebView_->GetPageAction (IWebView::PageAction::Back);
		Back_->setParent (this);

		BackMenu_ = new QMenu ();

		Forward_ = WebView_->GetPageAction (IWebView::PageAction::Forward);
		Forward_->setParent (this);

		ForwardMenu_ = new QMenu ();

		Reload_ = WebView_->GetPageAction (IWebView::PageAction::Reload);
		Stop_ = WebView_->GetPageAction (IWebView::PageAction::Stop);

		ReloadStop_ = new QAction (this);
		handleLoadProgress (0);

		ReloadPeriodically_ = new QAction (tr ("Reload periodically"), this);
		ReloadPeriodically_->setCheckable (true);
		ReloadPeriodically_->setProperty ("ActionIcon", "view-refresh-periodically");

		NotifyWhenFinished_ = new QAction (tr ("Notify when finished loading"), this);
		NotifyWhenFinished_->setCheckable (true);
		NotifyWhenFinished_->setProperty ("ActionIcon", "preferences-desktop-notification");
		NotifyWhenFinished_->setChecked (XmlSettingsManager::Instance ().property ("NotifyFinishedByDefault").toBool ());

		Add2Favorites_ = new QAction (tr ("Bookmark..."), this);
		Add2Favorites_->setProperty ("ActionIcon", "bookmark-new");
		Add2Favorites_->setEnabled (false);

		new UrlEditButtonsManager (WebView_.get (),
				Ui_.URLFrame_->GetEditAsProgressLine (),
				Add2Favorites_);

		Find_ = new QAction (tr ("Find..."),
				this);
		Find_->setProperty ("ActionIcon", "edit-find");
		Find_->setEnabled (false);

		Print_ = new QAction (this);
		Print_->setEnabled (false);

		PrintPreview_ = new QAction (this);
		PrintPreview_->setEnabled (false);

		ScreenSave_ = new QAction (this);
		ScreenSave_->setEnabled (false);

		ViewSources_ = new QAction (this);
		ViewSources_->setEnabled (false);

		SavePage_ = new QAction (tr ("Save page..."),
				this);
		SavePage_->setProperty ("ActionIcon", "download");
		SavePage_->setEnabled (false);
		SavePage_->setShortcut (QKeySequence (tr ("Ctrl+S")));

		ZoomIn_ = new QAction (this);
		ZoomOut_ = new QAction (this);
		ZoomReset_ = new QAction (this);

		HistoryAction_ = new QAction (tr ("Open history"),
				this);
		HistoryAction_->setCheckable (true);
		HistoryAction_->setShortcut (QKeySequence (tr ("Ctrl+H")));
		HistoryAction_->setProperty ("ActionIcon", "view-history");

		BookmarksAction_ = new QAction (tr ("Open bookmarks"),
				this);
		BookmarksAction_->setCheckable (true);
		BookmarksAction_->setShortcut (QKeySequence (tr ("Ctrl+B")));
		BookmarksAction_->setProperty ("ActionIcon", "bookmarks-organize");

		auto backButton = new QToolButton ();
		backButton->setMenu (BackMenu_);
		backButton->setDefaultAction (Back_);
		backButton->setPopupMode (QToolButton::MenuButtonPopup);
		ToolBar_->addWidget (backButton);

		auto fwdButton = new QToolButton ();
		fwdButton->setMenu (ForwardMenu_);
		fwdButton->setDefaultAction (Forward_);
		fwdButton->setPopupMode (QToolButton::MenuButtonPopup);
		ToolBar_->addWidget (fwdButton);

		ToolBar_->addAction (ReloadStop_);

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		QMenu *moreMenu = new QMenu (this);
		emit hookMoreMenuFillBegin (proxy, moreMenu, this);
		if (!proxy->IsCancelled ())
		{
			const QString tools = "Poshuku";
			WindowMenus_ [tools] << Find_;
			WindowMenus_ [tools] << Util::CreateSeparator (this);
			WindowMenus_ [tools] << Add2Favorites_;
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
		proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookMoreMenuFillEnd (proxy, moreMenu, this);

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
				SIGNAL (tabRecoverDataChanged ()));
		connect (Add2Favorites_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAdd2Favorites ()));
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { WebView_->Print (false); },
			Print_,
			SIGNAL (triggered ()),
			Print_
		};
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { WebView_->Print (true); },
			PrintPreview_,
			SIGNAL (triggered ()),
			PrintPreview_
		};
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

		const auto fullZoomer = new Zoomer
		{
			[this] { return WebView_->GetZoomFactor (); },
			[this] (qreal f) { WebView_->SetZoomFactor (f); },
			this
		};
		fullZoomer->InstallScrollFilter (webViewWidget,
				[] (QWheelEvent *ev) { return ev->modifiers () == Qt::ControlModifier; });
		fullZoomer->SetActionsTriple (ZoomIn_, ZoomOut_, ZoomReset_);

		connect (Ui_.URLFrame_,
				SIGNAL (load (QString)),
				this,
				SLOT (handleURLFrameLoad (QString)));

		connect (webViewWidget,
				SIGNAL (titleChanged (QString)),
				this,
				SLOT (updateTitle (QString)));
		connect (webViewWidget,
				SIGNAL (titleChanged (QString)),
				this,
				SLOT (updateLogicalPath ()));
		connect (webViewWidget,
				SIGNAL (urlChanged (QUrl)),
				this,
				SLOT (handleUrlChanged (QUrl)));
		connect (webViewWidget,
				SIGNAL (urlChanged (QUrl)),
				this,
				SLOT (updateLogicalPath ()));
		connect (webViewWidget,
				SIGNAL (loadProgress (int)),
				this,
				SLOT (handleLoadProgress (int)));
		connect (webViewWidget,
				SIGNAL (loadProgress (int)),
				this,
				SLOT (handleIconChanged ()));
		connect (webViewWidget,
				SIGNAL (iconChanged ()),
				this,
				SLOT (handleIconChanged ()));
		connect (webViewWidget,
				SIGNAL (statusBarMessage (QString)),
				this,
				SLOT (handleStatusBarMessage (QString)),
				Qt::QueuedConnection);
		connect (webViewWidget,
				SIGNAL (linkHovered (QString, QString, QString)),
				this,
				SLOT (handleStatusBarMessage (QString)),
				Qt::QueuedConnection);
		connect (webViewWidget,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (setScrollPosition ()));
		connect (webViewWidget,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (notifyLoadFinished (bool)));
		connect (webViewWidget,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleIconChanged ()));
		connect (webViewWidget,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (checkLoadedDocument ()));
		connect (webViewWidget,
				SIGNAL (loadStarted ()),
				this,
				SLOT (enableActions ()));
		connect (webViewWidget,
				SIGNAL (loadStarted ()),
				this,
				SLOT (updateNavHistory ()),
				Qt::QueuedConnection);
		connect (webViewWidget,
				SIGNAL (closeRequested ()),
				this,
				SIGNAL (removeTab ()));

		connect (this,
				&BrowserWidget::removeTab,
				this,
				&QObject::deleteLater);

		connect (HistoryAction_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleShortcutHistory ()));

		connect (BookmarksAction_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleShortcutBookmarks ()));

		RememberDialog_ = new PasswordRemember (WebView_->GetQWidget ());
		RememberDialog_->hide ();

		connect (webViewWidget,
				SIGNAL (storeFormData (PageFormsData_t)),
				RememberDialog_,
				SLOT (add (PageFormsData_t)));

		updateLogicalPath ();

		RegisterShortcuts (sm);

		new BrowserWidgetSettingsHandler { this };

		WebView_->SurroundingsInitialized ();

		if (Kind_ == Kind::Own)
		{
			new FocusTracker { *Ui_.URLFrame_->GetEdit (), WebView_, *this };
			emit hookBrowserWidgetInitialized (std::make_shared<Util::DefaultHookProxy> (), this);
		}
	}

	BrowserWidget::~BrowserWidget ()
	{
		if (Kind_ == Kind::Own)
			Core::Instance ().Unregister (this);

		delete ToolBar_;
	}

	void BrowserWidget::SetParentMultiTabs (QObject *parent)
	{
		S_MultiTabsParent_ = parent;
	}

	QLineEdit* BrowserWidget::GetURLEdit () const
	{
		return Ui_.URLFrame_->GetEdit ();
	}

	IWebView* BrowserWidget::GetWebView () const
	{
		return WebView_.get ();
	}

	void BrowserWidget::InsertFindAction (QMenu *menu, const QString& text)
	{
		Find_->setData (text);
		menu->addAction (Find_);

		const auto searchAct = menu->addAction (tr ("Search..."));
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[text, this]
			{
				const auto st = new SearchText (text, Proxy_, this);
				st->setAttribute (Qt::WA_DeleteOnClose);
				st->show ();
			},
			searchAct,
			SIGNAL (triggered ()),
			searchAct
		};

		new Util::StdDataFilterMenuCreator (text, Proxy_->GetEntityManager (), menu);
	}

	void BrowserWidget::AddStandardActions (QMenu *menu)
	{
		menu->addAction (Add2Favorites_);
		menu->addSeparator ();
		menu->addAction (Print_);
		menu->addAction (PrintPreview_);
		menu->addSeparator ();
		menu->addAction (ViewSources_);
		menu->addAction (SavePage_);
		menu->addAction (ScreenSave_);
		menu->addSeparator ();
		menu->addAction (ReloadPeriodically_);
		menu->addAction (NotifyWhenFinished_);
		menu->addSeparator ();
		menu->addAction (ChangeEncoding_->menuAction ());
	}

	QObject* BrowserWidget::GetQObject ()
	{
		return this;
	}

	BrowserWidgetSettings BrowserWidget::GetWidgetSettings () const
	{
		QByteArray ba;
		QDataStream out (&ba, QIODevice::WriteOnly);
		WebView_->GetHistory ()->Save (out);
		return
		{
			WebView_->GetZoomFactor (),
			NotifyWhenFinished_->isChecked (),
			ReloadPeriodically_->isChecked () ?
				QTime (0, 0, 0).addMSecs (ReloadTimer_->interval ()) :
				QTime (0, 0, 0),
			ba,
			WebView_->GetScrollPosition (),
			WebView_->GetDefaultTextEncoding ()
		};
	}

	void BrowserWidget::SetWidgetSettings (const BrowserWidgetSettings& settings)
	{
		if (std::fabs (settings.ZoomFactor_ - 1) >
				std::numeric_limits<decltype (settings.ZoomFactor_)>::epsilon ())
			WebView_->SetZoomFactor (settings.ZoomFactor_);

		NotifyWhenFinished_->setChecked (settings.NotifyWhenFinished_);
		QTime interval = settings.ReloadInterval_;
		QTime null (0, 0, 0);
		int msecs = null.msecsTo (interval);
		if (msecs >= 1000)
		{
			ReloadPeriodically_->setChecked (true);
			SetActualReloadInterval (interval);
		}

		if (!settings.WebHistorySerialized_.isEmpty ())
			QTimer::singleShot (0, this,
					[this, hist = settings.WebHistorySerialized_]
					{
						QDataStream str { hist };
						WebView_->GetHistory ()->Load (str);
					});

		if (!settings.ScrollPosition_.isNull ())
			SetOnLoadScrollPoint (settings.ScrollPosition_);

		WebView_->SetDefaultTextEncoding (settings.DefaultEncoding_);
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
			WebView_->Load (url, {});
		}
	}

	void BrowserWidget::Load (const QUrl& url)
	{
		SetURL (url);
	}

	void BrowserWidget::SetHtml (const QString& html, const QUrl& base)
	{
		Ui_.URLFrame_->GetEdit ()->clear ();
		HtmlMode_ = true;
		WebView_->SetContent (html.toUtf8 (), "text/html;charset=UTF-8", base);
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

	QWidget* BrowserWidget::GetQWidget ()
	{
		return this;
	}

	void BrowserWidget::Remove ()
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookTabRemoveRequested (proxy, this);
		if (proxy->IsCancelled ())
			return;

		emit removeTab ();
	}

	QToolBar* BrowserWidget::GetToolBar () const
	{
		return Kind_ == Kind::Own ? ToolBar_ : 0;
	}

	namespace
	{
		void Append (QList<QAction*>& result, const QList<QObject*>& objs)
		{
			for (const auto obj : objs)
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
					<< Print_
					<< Back_;

		plugResult.clear ();
		proxy->FillValue ("endActions", plugResult);
		Append (result, plugResult);

		return result;
	}

	QMap<QString, QList<QAction*>> BrowserWidget::GetWindowMenus () const
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

	void BrowserWidget::FillMimeData (QMimeData *data)
	{
		const auto& url = WebView_->GetUrl ();
		if (!url.isEmpty () && url.isValid ())
			data->setUrls ({ url });

		const auto widget = WebView_->GetQWidget ();

		QImage image { widget->size (), QImage::Format_ARGB32 };
		widget->render (&image);
		data->setImageData (image);
	}

	void BrowserWidget::HandleDragEnter (QDragMoveEvent *event)
	{
		if (event->mimeData ()->hasUrls ())
			event->acceptProposedAction ();
	}

	void BrowserWidget::HandleDrop (QDropEvent *event)
	{
		const auto& urls = event->mimeData ()->urls ();
		if (!urls.isEmpty ())
			SetURL (urls.first ());
		event->acceptProposedAction ();
	}

	void BrowserWidget::SetTabRecoverData (const QByteArray& data)
	{
		QUrl url;
		BrowserWidgetSettings settings;

		QDataStream str (data);
		str >> url
				>> settings;

		SetURL (url);
		SetWidgetSettings (settings);
	}

	QByteArray BrowserWidget::GetTabRecoverData () const
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		str << WebView_->GetUrl ();
		str << GetWidgetSettings ();
		return result;
	}

	QString BrowserWidget::GetTabRecoverName () const
	{
		return QString ("%1 (%2)")
				.arg (WebView_->GetTitle ())
				.arg (WebView_->GetUrl ().toString ());
	}

	QIcon BrowserWidget::GetTabRecoverIcon () const
	{
		return WebView_->GetIcon ();
	}

	void BrowserWidget::SetFontFamily (FontFamily family, const QFont& font)
	{
		if (const auto iwfs = qobject_cast<IWkFontsSettable*> (WebView_->GetQWidget ()))
			iwfs->SetFontFamily (family, font);
	}

	void BrowserWidget::SetFontSize (FontSize type, int size)
	{
		if (const auto iwfs = qobject_cast<IWkFontsSettable*> (WebView_->GetQWidget ()))
			iwfs->SetFontSize (type, size);
	}

	void BrowserWidget::SetOnLoadScrollPoint (const QPoint& sp)
	{
		OnLoadPos_ = sp;
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
		emit hookIconChanged (proxy, this);
		if (proxy->IsCancelled ())
			return;

		auto icon = WebView_->GetIcon ();
		if (icon.isNull ())
			icon = Core::Instance ().GetIcon (WebView_->GetUrl ());

		Ui_.URLFrame_->SetFavicon (icon);

		emit changeTabIcon (icon);
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
			LinkTextItem_->hide ();
			return;
		}

		const auto webViewWidget = WebView_->GetQWidget ();

		const auto& metrics = LinkTextItem_->fontMetrics ();
		msg = metrics.elidedText (msg, Qt::ElideMiddle, webViewWidget->rect ().width () * 5 / 11);
		const auto margin = LinkTextItem_->margin ();
		LinkTextItem_->setFixedSize (metrics.horizontalAdvance (msg) + 2 * margin, metrics.height () + 2 * margin);
		LinkTextItem_->setText (msg);

		const auto& localCursorPos = webViewWidget->mapFromGlobal (QCursor::pos ());

		const int textHeight = metrics.boundingRect (msg).height ();
		const qreal y = webViewWidget->rect ().height () - textHeight - 7;
		const qreal x = QRect (QPoint (0, y), LinkTextItem_->size ()).contains (localCursorPos) ?
				webViewWidget->rect ().width () - LinkTextItem_->width () + margin :
				margin;
		LinkTextItem_->move (x, y);
		LinkTextItem_->show ();
		LinkTextItem_->raise ();
	}

	void BrowserWidget::handleURLFrameLoad (QString text)
	{
		const auto& proxy = std::make_shared<Util::DefaultHookProxy> ();
		proxy->SetValue ("Text", text);
		emit hookURLEditReturnPressed (proxy, this);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("Text", text);
		Load (Core::Instance ().MakeURL (text));
	}

	void BrowserWidget::handleReloadPeriodically ()
	{
		if (ReloadPeriodically_->isChecked ())
		{
			ReloadIntervalSelector sel { this };
			if (sel.exec () != QDialog::Accepted)
			{
				ReloadPeriodically_->setChecked (false);
				ReloadPeriodically_->setStatusTip ({});
				ReloadPeriodically_->setToolTip ({});
				ReloadTimer_->stop ();
				return;
			}

			const auto& value = sel.GetInterval ();
			const QTime null { 0, 0, 0 };
			const auto msecs = null.msecsTo (value);
			if (msecs < 1000)
			{
				ReloadPeriodically_->setChecked (false);
				ReloadPeriodically_->setStatusTip ({});
				ReloadPeriodically_->setToolTip ({});
				ReloadTimer_->stop ();
				return;
			}

			SetActualReloadInterval (value);
		}
		else if (ReloadTimer_->isActive ())
		{
			ReloadPeriodically_->setStatusTip ({});
			ReloadPeriodically_->setToolTip ({});
			ReloadTimer_->stop ();
		}

		emit tabRecoverDataChanged ();
	}

	void BrowserWidget::handleAdd2Favorites ()
	{
		const auto& url = WebView_->GetUrl ().toString ();

		if (Core::Instance ().IsUrlInFavourites (url))
			Core::Instance ().GetFavoritesModel ()->removeItem (url);
		else
			emit addToFavorites (WebView_->GetTitle (), url);
	}

	void BrowserWidget::handleFind ()
	{
		const auto act = qobject_cast<QAction*> (sender ());
		WebView_->InitiateFind (act ? act->data ().toString () : QString {});
	}

	void BrowserWidget::handleScreenSave ()
	{
		ScreenShotSaveDialog dia (WebView_->MakeFullPageSnapshot (), this);
		dia.exec ();
	}

	void BrowserWidget::handleViewSources ()
	{
		WebView_->ToHtml ([this] (const QString& html)
				{
					auto e = Util::MakeEntity (html,
							{},
							FromUserInitiated,
							"x-leechcraft/plain-text-document");
					e.Additional_ ["Language"] = "HTML";
					e.Additional_ ["IsTemporaryDocument"] = true;

					if (Proxy_->GetEntityManager ()->HandleEntity (e))
						return;

					auto viewer = new SourceViewer (this);
					viewer->setAttribute (Qt::WA_DeleteOnClose);
					viewer->SetHtml (html);
					viewer->show ();
				});
	}

	void BrowserWidget::handleSavePage ()
	{
		Entity e = Util::MakeEntity (WebView_->GetUrl (),
				QString (),
				FromUserInitiated);
		e.Additional_ ["AllowedSemantics"] = QStringList { "fetch", "save" };
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	QLineEdit* BrowserWidget::getAddressBar () const
	{
		return Ui_.URLFrame_->GetEdit ();
	}

	QWidget* BrowserWidget::getSideBar () const
	{
		return Ui_.Sidebar_;
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

	void BrowserWidget::updateTitle (QString title)
	{
		if (title.isEmpty ())
			title = WebView_->GetTitle ();
		if (title.isEmpty ())
			title = QFileInfo { WebView_->GetUrl ().path () }.fileName ();

		emit changeTabName (title);
	}

	const int MaxHistoryItems = 10;

	namespace
	{
		void FillNavMenu (QMenu *menu, const QList<IWebViewHistory::IItem_ptr>& items)
		{
			menu->clear ();

			for (const auto& item : items)
			{
				if (!item->IsValid ())
					continue;

				const auto& url = item->GetUrl ();
				auto icon = item->GetIcon ();
				if (icon.isNull ())
					icon = Core::Instance ().GetIcon (url);
				auto act = menu->addAction (icon, item->GetTitle ());
				act->setToolTip (url.toString ());

				new Util::SlotClosure<Util::NoDeletePolicy>
				{
					[item] { item->Navigate (); },
					act,
					SIGNAL (triggered ()),
					act
				};
			}
		}
	}

	void BrowserWidget::updateNavHistory ()
	{
		const auto& history = WebView_->GetHistory ();

		auto backItems = history->GetItems (IWebViewHistory::Direction::Backward, MaxHistoryItems);
		std::reverse (backItems.begin (), backItems.end ());
		FillNavMenu (BackMenu_, backItems);

		FillNavMenu (ForwardMenu_,
				history->GetItems (IWebViewHistory::Direction::Forward, MaxHistoryItems));
	}

	namespace
	{
		class HtmlWriter
		{
			QXmlStreamWriter& W_;
		public:
			HtmlWriter (QXmlStreamWriter& w)
			: W_ (w)
			{
			}

			void WriteColored (const QString& color, const QString& text, int pad)
			{
				const auto& padStr = pad ?
						("; margin-left: " + QString::number (pad) + "em;") :
						QString ();

				W_.writeStartElement ("span");
				W_.writeAttribute ("style", "color:" + color + padStr);
				W_.writeCharacters (text);
				W_.writeEndElement ();
			}

			void ToHtml (const QDomDocument& doc)
			{
				const auto& doctype = doc.doctype ();
				if (!doctype.name ().isEmpty ())
				{
					W_.writeStartElement ("div");
					W_.writeAttribute ("style", "color: #964B00; margin-left: 1em;");
					W_.writeCharacters ("<!DOCTYPE ");

					WriteColored ("#000000", doctype.name (), 0);
					W_.writeCharacters (" ");
					WriteColored ("#881280", '"' + doctype.publicId () + '"', 0);
					W_.writeEmptyElement ("br");
					WriteColored ("#881280", '"' + doctype.systemId () + '"', 2);

					W_.writeCharacters (">");

					W_.writeEndElement ();
				}

				ToHtmlChildren (doc, [] {});
			}

			void ToHtml (const QDomElement& elem)
			{
				W_.writeStartElement ("div");
				W_.writeAttribute ("style", "color: #881280; margin-left: 1em;");
				W_.writeCharacters ("<" + elem.tagName ());

				const auto& attrs = elem.attributes ();
				for (int i = 0; i < attrs.size (); ++i)
				{
					const auto& attrNode = attrs.item (i).toAttr ();

					WriteColored ("#994500", " " + attrNode.name (), 0);
					WriteColored ("#881280", "=\"", 0);
					WriteColored ("#1A1AA6", attrNode.value (), 0);
					WriteColored ("#881280", "\"", 0);
				}

				bool hasChildren = false;
				auto childrenize = [this, &hasChildren]
				{
					if (hasChildren)
						return;

					hasChildren = true;
					W_.writeCharacters (">");
					W_.writeEmptyElement ("br");
				};

				ToHtmlChildren (elem, childrenize);

				W_.writeCharacters (hasChildren ? ("</" + elem.tagName () + ">") : " />");
				W_.writeEndElement ();
			}

			template<typename ChildrenizeCbType>
			void ToHtmlChildren (const QDomNode& elem, const ChildrenizeCbType& childrenize)
			{
				auto child = elem.firstChild ();
				while (!child.isNull ())
				{
					switch (child.nodeType ())
					{
					case QDomDocument::TextNode:
						childrenize ();
						WriteColored ("#000000", child.toText ().data (), 1);
						W_.writeEmptyElement ("br");
						break;
					case QDomDocument::CDATASectionNode:
						childrenize ();
						WriteColored ("#000000", "<![CDATA[ " + child.toCDATASection ().data () + " ]]>", 1);
						W_.writeEmptyElement ("br");
						break;
					case QDomDocument::CommentNode:
						childrenize ();
						WriteColored ("#00BB00", "<!--" + child.toComment ().data () + "-->", 1);
						W_.writeEmptyElement ("br");
						break;
					case QDomNode::ElementNode:
						childrenize ();
						ToHtml (child.toElement ());
						break;
					case QDomNode::ProcessingInstructionNode:
					{
						childrenize ();
						const auto& instr = child.toProcessingInstruction ();
						W_.writeStartElement ("div");
						W_.writeAttribute ("style", "color: #007700; margin-left: 1em;");
						W_.writeCharacters ("<?" + instr.target () + " ");
						WriteColored ("#994500", instr.data (), 0);
						W_.writeCharacters ("?>");
						W_.writeEndElement ();
						break;
					}
					case QDomNode::DocumentTypeNode:
					case QDomNode::EntityNode:
					case QDomNode::EntityReferenceNode:
					case QDomNode::AttributeNode:
					case QDomNode::CharacterDataNode:
					case QDomNode::BaseNode:
					case QDomNode::DocumentFragmentNode:
					case QDomNode::DocumentNode:
					case QDomNode::NotationNode:
						qWarning () << "unexpected node type"
								<< child.nodeType ();
						break;
					}
					child = child.nextSibling ();
				}
			}
		};
	}

	void BrowserWidget::checkLoadedDocument ()
	{
		WebView_->ToHtml ([this] (const QString& html)
				{
					QDomDocument doc;
					if (!doc.setContent (html))
						return;

					const auto& rootTagName = doc.documentElement ().tagName ().toLower ();
					if (rootTagName == "html" || rootTagName == "xhtml" || rootTagName == "svg")
						return;

					Entity e;
					e.Entity_ = WebView_->GetUrl ();
					e.Mime_ = "text/xml";
					e.Parameters_ = FromUserInitiated |
									OnlyHandle;
					e.Additional_ [IgnoreSelf] = true;
					e.Additional_ ["URLData"] = html;
					Proxy_->GetEntityManager ()->HandleEntity (e);

					QString formatted;
					QXmlStreamWriter w (&formatted);
					w.writeStartDocument ();
					w.writeStartElement ("html");
					w.writeStartElement ("head");
					w.writeTextElement ("title", WebView_->GetUrl ().toString ().toHtmlEscaped ());
					w.writeEndElement ();
					w.writeStartElement ("body");
					w.writeAttribute ("style", "font-family:monospace;");
					HtmlWriter { w }.ToHtml (doc);
					w.writeEndElement ();
					w.writeEndElement ();
					w.writeEndDocument ();

					WebView_->SetContent (formatted.toUtf8 (), "text/html");
				});
	}

	namespace
	{
		const QRegExp UrlInText (R"(://|www\.|\w\.\w)");

		void SavePixmap (const QPixmap& px, const QUrl& url, IEntityManager *iem)
		{
			if (px.isNull ())
				return;

			const auto origName = url.scheme () == "data" ?
					QString {} :
					QFileInfo { url.path () }.fileName ();

			QString filter;
			auto fname = QFileDialog::getSaveFileName (0,
					BrowserWidget::tr ("Save pixmap"),
					QDir::homePath () + '/' + origName,
					BrowserWidget::tr ("PNG image (*.png);;JPG image (*.jpg);;All files (*.*)"),
					&filter);

			if (fname.isEmpty ())
				return;

			if (QFileInfo { fname }.suffix ().isEmpty ())
			{
				if (filter.contains ("png"))
					fname += ".png";
				else if (filter.contains ("jpg"))
					fname += ".jpg";
			}

			QFile file { fname };
			if (!file.open (QIODevice::WriteOnly))
			{
				iem->HandleEntity (Util::MakeNotification ("Poshuku",
						BrowserWidget::tr ("Unable to save the image. Unable to open file for writing: %1.")
							.arg (file.errorString ()),
						Priority::Critical));
				return;
			}

			const auto& suf = QFileInfo { fname }.suffix ();
			const bool isLossless = suf.toLower () == "png";
			px.save (&file,
					suf.toUtf8 ().constData (),
					isLossless ? 0 : 100);
		}
	}

	void BrowserWidget::handleContextMenu (const QPoint& point, const ContextMenuInfo& info)
	{
		QPointer<QMenu> menu (new QMenu ());

		const auto iem = Proxy_->GetEntityManager ();

		IHookProxy_ptr proxy (new Util::DefaultHookProxy ());

		emit hookWebViewContextMenu (proxy, WebView_.get (), info, menu, WVSStart);

		auto addAction = [menu] (const QString& text, auto handler)
		{
			auto act = menu->addAction (text);
			new Util::SlotClosure<Util::DeleteLaterPolicy>
			{
				handler,
				act,
				SIGNAL (triggered ()),
				act
			};
			return act;
		};
		auto addWebAction = [menu, this] (IWebView::PageAction act)
		{
			if (const auto actObj = WebView_->GetPageAction (act))
				menu->addAction (actObj);
		};

		const auto raiseNewTabs = !XmlSettingsManager::Instance ().property ("BackgroundNewTabs").toBool ();
		if (!info.LinkUrl_.isEmpty ())
		{
			if (XmlSettingsManager::Instance ().property ("TryToDetectRSSLinks").toBool ())
			{
				bool hasAtom = info.LinkText_.contains ("Atom");
				bool hasRSS = info.LinkText_.contains ("RSS");

				if (hasAtom || hasRSS)
				{
					LC::Entity e;
					if (hasAtom)
					{
						e.Additional_ ["UserVisibleName"] = "Atom";
						e.Mime_ = "application/atom+xml";
					}
					else
					{
						e.Additional_ ["UserVisibleName"] = "RSS";
						e.Mime_ = "application/rss+xml";
					}

					e.Entity_ = info.LinkUrl_;
					e.Parameters_ = LC::FromUserInitiated |
							LC::OnlyHandle;

					if (iem->CouldHandle (e))
						addAction (tr ("Subscribe"), [iem, e] { iem->HandleEntity (e); });
				}
			}

			addAction (tr ("Open &here"), [&] { SetURL (info.LinkUrl_); });
			addAction (tr ("Open in new &tab"),
					[&] { Core::Instance ().MakeWebView (raiseNewTabs)->Load (info.LinkUrl_); });
			menu->addSeparator ();
			addWebAction (IWebView::PageAction::DownloadLinkToDisk);

			addAction (tr ("&Bookmark link..."),
					[&] { emit addToFavorites (info.LinkText_, info.LinkUrl_.toString ()); });

			menu->addSeparator ();
			if (!info.SelectedPageText_.isEmpty ())
				addWebAction (IWebView::PageAction::Copy);
			addWebAction (IWebView::PageAction::CopyLinkToClipboard);
			addWebAction (IWebView::PageAction::InspectElement);
		}
		else if (info.SelectedPageText_.contains (UrlInText))
			addAction (tr ("Open as link"),
					[&]
					{
						const auto& url = QUrl::fromUserInput (info.SelectedPageText_);
						Core::Instance ().MakeWebView (raiseNewTabs)->Load (url);
					});

		emit hookWebViewContextMenu (proxy, WebView_.get (), info, menu, WVSAfterLink);

		if (!info.ImageUrl_.isEmpty ())
		{
			if (!menu->isEmpty ())
				menu->addSeparator ();
			addAction (tr ("Open image here"), [&] { SetURL (info.ImageUrl_); });
			addWebAction (IWebView::PageAction::OpenImageInNewWindow);
			menu->addSeparator ();
			addWebAction (IWebView::PageAction::DownloadImageToDisk);

			const auto pxAct = addAction (tr ("Save pixmap..."),
					[&] { SavePixmap (info.ImagePixmap_, info.ImageUrl_, iem); });
			pxAct->setToolTip (tr ("Saves the rendered pixmap without redownloading."));

			addWebAction (IWebView::PageAction::CopyImageToClipboard);
			addWebAction (IWebView::PageAction::CopyImageUrlToClipboard);
		}

		emit hookWebViewContextMenu (proxy, WebView_.get (), info, menu, WVSAfterImage);

		bool hasSelected = !info.SelectedPageText_.isEmpty ();
		if (hasSelected)
		{
			if (!menu->isEmpty ())
				menu->addSeparator ();
			menu->addAction (WebView_->GetPageAction (IWebView::PageAction::Copy));
		}

		if (info.IsContentEditable_)
			menu->addAction (WebView_->GetPageAction (IWebView::PageAction::Paste));


		if (hasSelected)
			InsertFindAction (menu, info.SelectedPageText_);

		emit hookWebViewContextMenu (proxy, WebView_.get (), info, menu, WVSAfterSelectedText);

		if (menu->isEmpty ())
			menu = WebView_->CreateStandardContextMenu ();

		addWebAction (IWebView::PageAction::ReloadAndBypassCache);
		if (!menu->isEmpty ())
			menu->addSeparator ();

		AddStandardActions (menu);

		emit hookWebViewContextMenu (proxy, WebView_.get (), info, menu, WVSAfterFinish);

		menu->exec (point);

		delete menu;
	}

	void BrowserWidget::setScrollPosition ()
	{
		if (!OnLoadPos_.isNull ())
		{
			WebView_->SetScrollPosition (OnLoadPos_);
			OnLoadPos_ = QPoint ();
		}
	}

	void BrowserWidget::handleLoadProgress (int p)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookLoadProgress (proxy, this, p);
		if (proxy->IsCancelled ())
			return;

		proxy->FillValue ("progress", p);

		QString title = WebView_->GetTitle ();
		if (title.isEmpty ())
			title = QFileInfo (WebView_->GetUrl ().path ()).fileName ();

		if (p > 0 && p < 100)
			title.prepend (QString ("[%1%] ").arg (p));
		emit changeTabName (title);

		QAction *o = 0;
		QAction *n = 0;
		QString actionIcon;
		if (p < 100 && p > 0)
		{
			o = Reload_;
			n = Stop_;
			actionIcon = "process-stop";
		}
		else
		{
			o = Stop_;
			n = Reload_;
			actionIcon = "view-refresh";
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
				this,
				ok,
				NotifyWhenFinished_->isChecked (),
				Kind_ == Kind::Own,
				HtmlMode_);

		proxy->FillValue ("ok", ok);

		if (!NotifyWhenFinished_->isChecked () ||
			Kind_ != Kind::Own ||
			HtmlMode_ ||
			isVisible ())
			return;

		QString h = WebView_->GetTitle ();
		if (h.isEmpty ())
			h = WebView_->GetUrl ().toString ();
		if (h.isEmpty ())
			return;

		QString text;
		Priority prio = Priority::Info;

		const auto& escapedTitle = WebView_->GetTitle ().toHtmlEscaped ();
		if (ok)
			text = tr ("Page load finished: %1")
					.arg (escapedTitle);
		else
		{
			text = tr ("Page load failed: %1")
					.arg (escapedTitle);
			prio = Priority::Warning;
		}

		Entity e = Util::MakeNotification ("Poshuku", text, prio);
		Util::NotificationActionHandler *nh = new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open"), [this] () { emit raiseTab (); });
		nh->AddDependentObject (this);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void BrowserWidget::handleChangeEncodingAboutToShow ()
	{
		ChangeEncoding_->clear ();

		QStringList codecs;
		QList<int> mibs = QTextCodec::availableMibs ();
		QMap<QString, int> name2mib;
		for (const auto mib : mibs)
		{
			const auto& name = QTextCodec::codecForMib (mib)->name ();
			codecs << name;
			name2mib [name] = mib;
		}
		codecs.sort ();

		const auto& defaultEncoding = WebView_->GetDefaultTextEncoding ();
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
		WebView_->SetDefaultTextEncoding (encoding);
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

		QUrl url = WebView_->GetUrl ();
		QString title = WebView_->GetTitle ();
		if (title.isEmpty ())
			title = tr ("No title");
		QString host = url.host ();
		host.remove ("www.");
		QStringList path;
		path << (host.isEmpty () ? QString ("Poshuku") : host);
		path << title;

		auto domains = host.split ('.', Qt::SkipEmptyParts);
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

	void BrowserWidget::handleUrlChanged (const QUrl& value)
	{
		auto userText = value.toString ();
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

			for (auto str : caps)
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

		if (userText.isEmpty ())
			return;

		if (value.scheme () != "data")
		{
			Ui_.URLFrame_->GetEdit ()->setText (userText);
			Ui_.URLFrame_->GetEdit ()->repaint ();
		}

		emit urlChanged (value);
	}

	namespace
	{
		QString GetFeatureText (IWebView::Feature feature)
		{
			switch (feature)
			{
			case IWebView::Feature::Notifications:
				return BrowserWidget::tr ("%1 requests access to notifications.");
			case IWebView::Feature::Geolocation:
				return BrowserWidget::tr ("%1 requests access to geolocation services.");
			}

			Util::Unreachable ();
		}
	}

	void BrowserWidget::handleFeaturePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr& origin,
			IWebView::Feature feature)
	{
		const auto& text = GetFeatureText (feature)
				.arg (origin->GetName ());
		qDebug () << Q_FUNC_INFO << WebView_->GetUrl () << text;

		const auto notification = new FeaturePermNotification { text, WebView_->GetQWidget () };
		notification->show ();

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[notification, origin]
			{
				origin->SetPermission (IWebView::Permission::Grant);
				notification->deleteLater ();
			},
			notification,
			SIGNAL (granted ()),
			notification
		};
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[notification, origin]
			{
				origin->SetPermission (IWebView::Permission::Deny);
				notification->deleteLater ();
			},
			notification,
			SIGNAL (denied ()),
			notification
		};
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
		int splitterSize = XmlSettingsManager::Instance ().Property ("HistoryBoormarksPanelSize", 250).toInt ();
		int wSize = WebView_->GetQWidget ()->size ().width ();

		if (!Ui_.Splitter_->sizes ().at (0))
		{
			Ui_.Splitter_->setSizes (QList<int> () << splitterSize << wSize - splitterSize);
			Ui_.Sidebar_->GetMainTabBar ()->setCurrentIndex (currentIndex);
		}
		else if (Ui_.Sidebar_->GetMainTabBar ()->currentIndex () != currentIndex)
			Ui_.Sidebar_->GetMainTabBar ()->setCurrentIndex (currentIndex);
		else
		{
			XmlSettingsManager::Instance ().setProperty ("HistoryBoormarksPanelSize", Ui_.Splitter_->sizes ().at (0));
			Ui_.Splitter_->setSizes ({ 0, wSize });
		}
	}

	void BrowserWidget::RegisterShortcuts (Util::ShortcutManager *sm)
	{
#define REG(n) sm->RegisterAction ("Browser" # n, n)
		REG (Cut_);
		REG (Copy_);
		REG (Paste_);
		REG (Back_);
		REG (Forward_);
		REG (Reload_);
		REG (Stop_);
		REG (Add2Favorites_);
		REG (Print_);
		REG (PrintPreview_);
		REG (ScreenSave_);
		REG (ViewSources_);
		REG (ZoomIn_);
		REG (ZoomOut_);
		REG (ZoomReset_);
#undef REG
	}
}
}
