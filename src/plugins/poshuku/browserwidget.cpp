#include "browserwidget.h"
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QtDebug>
#include <QToolBar>
#include <QBuffer>
#include <QDial>
#include <QMenu>
#include <QMovie>
#include <QWidgetAction>
#include <QCompleter>
#include <QPrinter>
#include <QPrintDialog>
#include <QTimer>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QWebFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QXmlStreamReader>
#include <QTextCodec>
#include "core.h"
#include "historymodel.h"
#include "finddialog.h"
#include "screenshotsavedialog.h"
#include "xmlsettingsmanager.h"
#include "sourceviewer.h"
#include "passwordremember.h"

using LeechCraft::ActionInfo;

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
, HtmlMode_ (false)
, Own_ (true)
{
	Ui_.setupUi (this);

	Cut_ = Ui_.WebView_->pageAction (QWebPage::Cut);
	Cut_->setProperty ("ActionIcon", "poshuku_cut");
	Copy_ = Ui_.WebView_->pageAction (QWebPage::Copy);
	Copy_->setProperty ("ActionIcon", "poshuku_copy");
	Paste_ = Ui_.WebView_->pageAction (QWebPage::Paste);
	Paste_->setProperty ("ActionIcon", "poshuku_paste");

	ToolBar_ = new QToolBar (this);
	
	Back_ = Ui_.WebView_->pageAction (QWebPage::Back);
	Back_->setParent (this);
	Back_->setProperty ("ActionIcon", "poshuku_back");

	Forward_ = Ui_.WebView_->pageAction (QWebPage::Forward);
	Forward_->setParent (this);
	Forward_->setProperty ("ActionIcon", "poshuku_forward");

	Reload_ = Ui_.WebView_->pageAction (QWebPage::Reload);
	Reload_->setParent (this);
	Reload_->setProperty ("ActionIcon", "poshuku_reload");

	Stop_ = Ui_.WebView_->pageAction (QWebPage::Stop);
	Stop_->setParent (this);
	Stop_->setProperty ("ActionIcon", "poshuku_stop");

	QMenu *moreMenu = new QMenu (this);
	QAction *more = moreMenu->menuAction ();
	more->setText (tr ("More..."));
	more->setProperty ("ActionIcon", "poshuku_more");

	Add2Favorites_ = new QAction (tr ("Add to favorites..."),
			this);
	Add2Favorites_->setProperty ("ActionIcon", "poshuku_addtofavorites");
	Add2Favorites_->setEnabled (false);

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

	NewTab_ = new QAction (tr ("Create new tab"),
			this);
	NewTab_->setProperty ("ActionIcon", "poshuku_newtab");

	CloseTab_ = new QAction (tr ("Close this tab"),
			this);
	CloseTab_->setProperty ("ActionIcon", "poshuku_closetab");

	ZoomIn_ = new QAction (tr ("Zoom in"),
			this);
	ZoomIn_->setProperty ("ActionIcon", "poshuku_zoomin");

	ZoomOut_ = new QAction (tr ("Zoom out"),
			this);
	ZoomOut_->setProperty ("ActionIcon", "poshuku_zoomout");

	ZoomReset_ = new QAction (tr ("Reset zoom"),
			this);
	ZoomReset_->setProperty ("ActionIcon", "poshuku_zoomreset");

	ImportXbel_ = new QAction (tr ("Import XBEL..."),
			this);
	ImportXbel_->setProperty ("ActionIcon", "poshuku_importxbel");

	ExportXbel_ = new QAction (tr ("Export XBEL..."),
			this);
	ExportXbel_->setProperty ("ActionIcon", "poshuku_exportxbel");

	ToolBar_->addAction (Back_);
	ToolBar_->addAction (Forward_);
	ToolBar_->addAction (Reload_);
	ToolBar_->addAction (Stop_);
	ToolBar_->addAction (more);

	moreMenu->addAction (Find_);
	moreMenu->addAction (Add2Favorites_);
	moreMenu->addSeparator ();
	moreMenu->addAction (ZoomIn_);
	moreMenu->addAction (ZoomOut_);
	moreMenu->addAction (ZoomReset_);
	moreMenu->addSeparator ();
	moreMenu->addAction (Print_);
	moreMenu->addAction (PrintPreview_);
	moreMenu->addAction (ScreenSave_);
	moreMenu->addSeparator ();
	moreMenu->addAction (ViewSources_);
	moreMenu->addSeparator ();
	moreMenu->addAction (ImportXbel_);
	moreMenu->addAction (ExportXbel_);
	moreMenu->addSeparator ();

	RecentlyClosed_ = moreMenu->addMenu (tr ("Recently closed"));
	RecentlyClosed_->setEnabled (false);
	RecentlyClosedAction_ = RecentlyClosed_->menuAction ();

	moreMenu->addMenu (Core::Instance ().GetPluginsMenu ());

	ExternalLinks_ = new QMenu (this);
	ExternalLinks_->menuAction ()->setText (tr ("External links"));
	ExternalLinks_->menuAction ()->
		setProperty ("ActionIcon", "poshuku_externalentities");

	QWidgetAction *addressBar = new QWidgetAction (this);
	addressBar->setDefaultWidget (Ui_.URLEdit_);
	ToolBar_->addAction (addressBar);

	ToolBar_->addAction (NewTab_);
	ToolBar_->addAction (CloseTab_);

	static_cast<QVBoxLayout*> (layout ())->insertWidget (0, ToolBar_);

	connect (Ui_.WebView_,
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
	connect (NewTab_,
			SIGNAL (triggered ()),
			this,
			SLOT (handleNewTab ()));
	connect (CloseTab_,
			SIGNAL (triggered ()),
			this,
			SIGNAL (needToClose ()));
	connect (ZoomIn_,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (zoomIn ()));
	connect (ZoomOut_,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (zoomOut ()));
	connect (ZoomReset_,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (zoomReset ()));
	connect (ImportXbel_,
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (importXbel ()));
	connect (ExportXbel_,
			SIGNAL (triggered ()),
			&Core::Instance (),
			SLOT (exportXbel ()));

	connect (Ui_.WebView_,
			SIGNAL (titleChanged (const QString&)),
			this,
			SIGNAL (titleChanged (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (urlChanged (const QString&)),
			this,
			SIGNAL (urlChanged (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (urlChanged (const QString&)),
			Ui_.URLEdit_,
			SLOT (setText (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (loadProgress (int)),
			this,
			SLOT (handleLoadProgress (int)));
	connect (Ui_.WebView_,
			SIGNAL (loadProgress (int)),
			this,
			SLOT (handleIconChanged ()));
	connect (Ui_.WebView_,
			SIGNAL (iconChanged ()),
			this,
			SLOT (handleIconChanged ()));
	connect (Ui_.WebView_,
			SIGNAL (statusBarMessage (const QString&)),
			this,
			SLOT (handleStatusBarMessage (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
	connect (Ui_.WebView_->page (),
			SIGNAL (linkHovered (const QString&,
					const QString&,
					const QString&)),
			this,
			SLOT (handleStatusBarMessage (const QString&)));
	connect (Ui_.WebView_,
			SIGNAL (loadFinished (bool)),
			this,
			SLOT (handleLoadFinished ()));
	connect (Ui_.WebView_,
			SIGNAL (loadFinished (bool)),
			this,
			SLOT (updateTooltip ()));
	connect (Ui_.WebView_,
			SIGNAL (loadStarted ()),
			this,
			SLOT (enableActions ()));
	connect (Ui_.WebView_,
			SIGNAL (loadStarted ()),
			this,
			SLOT (setupLoading ()));
	connect (Ui_.WebView_,
			SIGNAL (printRequested (QWebFrame*)),
			this,
			SLOT (handleViewPrint (QWebFrame*)));
	connect (Ui_.WebView_,
			SIGNAL (closeRequested ()),
			this,
			SIGNAL (needToClose ()));

	connect (&Core::Instance (),
			SIGNAL (newUnclose (QAction*)),
			this,
			SLOT (handleNewUnclose (QAction*)));

	QCompleter *completer = new QCompleter (this);
	completer->setModel (Core::Instance ().GetURLCompletionModel ());
	Ui_.URLEdit_->setCompleter (completer);
	connect (Ui_.URLEdit_,
			SIGNAL (textEdited (const QString&)),
			Core::Instance ().GetURLCompletionModel (),
			SLOT (setBase (const QString&)));

	QTimer::singleShot (100,
			this,
			SLOT (focusLineEdit ()));

	FindDialog_ = new FindDialog (Ui_.WebFrame_);
	FindDialog_->hide ();

	connect (FindDialog_,
			SIGNAL (next (const QString&, QWebPage::FindFlags)),
			this,
			SLOT (findText (const QString&, QWebPage::FindFlags)));

	RememberDialog_ = new PasswordRemember (Ui_.WebFrame_);
	RememberDialog_->hide ();
	
	connect (Ui_.WebView_,
			SIGNAL (storeFormData (const PageFormsData_t&)),
			RememberDialog_,
			SLOT (add (const PageFormsData_t&)));
}

BrowserWidget::~BrowserWidget ()
{
	if (Own_)
		Core::Instance ().Unregister (this);
}

void BrowserWidget::Deown ()
{
	Own_ = false;
}

void BrowserWidget::InitShortcuts ()
{
	const IShortcutProxy *proxy = Core::Instance ().GetShortcutProxy ();
	QObject *object = Core::Instance ().parent ();

	Cut_->setShortcut (proxy->GetShortcut (object, EACut_));
	Copy_->setShortcut (proxy->GetShortcut (object, EACopy_));
	Paste_->setShortcut (proxy->GetShortcut (object, EAPaste_));
	Back_->setShortcut (proxy->GetShortcut (object, EABack_));
	Forward_->setShortcut (proxy->GetShortcut (object, EAForward_));
	Reload_->setShortcut (proxy->GetShortcut (object, EAReload_));
	Stop_->setShortcut (proxy->GetShortcut (object, EAStop_));
	Add2Favorites_->setShortcut (proxy->GetShortcut (object, EAAdd2Favorites_));
	Find_->setShortcut (proxy->GetShortcut (object, EAFind_));
	Print_->setShortcut (proxy->GetShortcut (object, EAPrint_));
	PrintPreview_->setShortcut (proxy->GetShortcut (object, EAPrintPreview_));
	ScreenSave_->setShortcut (proxy->GetShortcut (object, EAScreenSave_));
	ViewSources_->setShortcut (proxy->GetShortcut (object, EAViewSources_));
	NewTab_->setShortcut (proxy->GetShortcut (object, EANewTab_));
	CloseTab_->setShortcut (proxy->GetShortcut (object, EACloseTab_));
	ZoomIn_->setShortcut (proxy->GetShortcut (object, EAZoomIn_));
	ZoomOut_->setShortcut (proxy->GetShortcut (object, EAZoomOut_));
	ZoomReset_->setShortcut (proxy->GetShortcut (object, EAZoomReset_));
	ImportXbel_->setShortcut (proxy->GetShortcut (object, EAImportXbel_));
	ExportXbel_->setShortcut (proxy->GetShortcut (object, EAExportXbel_));
	RecentlyClosedAction_->setShortcut (proxy->GetShortcut (object, EARecentlyClosedAction_));
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

CustomWebView* BrowserWidget::GetView () const
{
	return Ui_.WebView_;
}

void BrowserWidget::SetURL (const QUrl& url)
{
	if (!url.isEmpty ())
	{
		HtmlMode_ = false;
		Ui_.WebView_->Load (url);
	}
}

void BrowserWidget::Load (const QString& url)
{
	SetURL (Core::Instance ().MakeURL (url));
}

void BrowserWidget::SetHtml (const QString& html, const QUrl& base)
{
	Ui_.URLEdit_->clear ();
	HtmlMode_ = true;
	Ui_.WebView_->setHtml (html, base);
}

QWidget* BrowserWidget::Widget ()
{
	return this;
}

#define _LC_MERGE(a) EA##a

#define _LC_SINGLE(a) \
	case _LC_MERGE(a): \
		a->setShortcut (shortcut); \
		break;

#define _LC_TRAVERSER(z,i,array) \
	_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))

#define _LC_EXPANDER(Names) \
	switch (name) \
	{ \
		BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names) \
	}
void BrowserWidget::SetShortcut (int name, const QKeySequence& shortcut)
{
	_LC_EXPANDER ((Add2Favorites_)
			(Find_)
			(Print_)
			(PrintPreview_)
			(ScreenSave_)
			(ViewSources_)
			(NewTab_)
			(CloseTab_)
			(ZoomIn_)
			(ZoomOut_)
			(ZoomReset_)
			(ImportXbel_)
			(ExportXbel_)
			(Cut_)
			(Copy_)
			(Paste_)
			(Back_)
			(Forward_)
			(Reload_)
			(Stop_)
			(RecentlyClosedAction_));
}

#define _L(a,b) result [EA##a] = ActionInfo (a->text (), \
		b, a->icon ())
QMap<int, ActionInfo> BrowserWidget::GetActionInfo () const
{
	QMap<int, ActionInfo> result;
	_L (Add2Favorites_, tr ("Ctrl+D"));
	_L (Find_, tr ("Ctrl+F"));
	_L (Print_, tr ("Ctrl+P"));
	_L (PrintPreview_, tr ("Ctrl+Shift+P"));
	_L (ScreenSave_, Qt::Key_F12);
	_L (ViewSources_, QKeySequence ());
	_L (NewTab_, tr ("Ctrl+T"));
	_L (CloseTab_, tr ("Ctrl+W"));
	_L (ZoomIn_, Qt::CTRL + Qt::Key_Plus);
	_L (ZoomOut_, Qt::CTRL + Qt::Key_Minus);
	_L (ZoomReset_, tr ("Ctrl+0"));
	_L (ImportXbel_, QKeySequence ());
	_L (ExportXbel_, QKeySequence ());
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
	emit needToClose ();
}

QToolBar* BrowserWidget::GetToolBar () const
{
	return Own_ ? ToolBar_ : 0;
}

void BrowserWidget::PrintImpl (bool preview, QWebFrame *frame)
{
	std::auto_ptr<QPrinter> printer (new QPrinter ());

	QPrintDialog *dialog = new QPrintDialog (printer.get (), this);
	dialog->setWindowTitle (tr ("Print web page"));
	if (!Ui_.WebView_->selectedText ().isEmpty ())
		dialog->addEnabledOption (QAbstractPrintDialog::PrintSelection);

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

void BrowserWidget::handleIconChanged ()
{
	int progress = Ui_.Progress_->value ();
	if (progress == 100)
	{
		emit iconChanged (Ui_.WebView_->icon ());
		Loading_.reset ();
	}
	else if (Loading_)
		emit iconChanged (QIcon (Loading_->currentPixmap ()));
	else
		emit iconChanged (QIcon ());
}

void BrowserWidget::handleStatusBarMessage (const QString& msg)
{
	emit statusBarChanged (msg);
}

void BrowserWidget::on_URLEdit__returnPressed ()
{
	if (Ui_.URLEdit_->IsCompleting ())
		return;

	Load (Ui_.URLEdit_->text ());
}

void BrowserWidget::handleAdd2Favorites ()
{
	emit addToFavorites (Ui_.WebView_->title (),
			Ui_.WebView_->url ().toString ());
}

void BrowserWidget::handleFind ()
{
	FindDialog_->show ();
}

void BrowserWidget::findText (const QString& text,
		QWebPage::FindFlags flags)
{
	bool found = Ui_.WebView_->page ()->findText (text, flags);
	FindDialog_->SetSuccessful (found);
}

void BrowserWidget::handleViewPrint (QWebFrame *frame)
{
	PrintImpl (false, frame);
}

void BrowserWidget::handlePrinting ()
{
	PrintImpl (false, Ui_.WebView_->page ()->mainFrame ());
}

void BrowserWidget::handlePrintingWithPreview ()
{
	PrintImpl (true, Ui_.WebView_->page ()->mainFrame ());
}

void BrowserWidget::handleScreenSave ()
{
	QSize contentsSize = Ui_.WebView_->page ()->mainFrame ()->contentsSize ();
	QSize oldSize = Ui_.WebView_->page ()->viewportSize ();
	QRegion clip (0, 0, contentsSize.width (), contentsSize.height ());

	QPixmap image (contentsSize);
	QPainter painter (&image);
	Ui_.WebView_->page ()->setViewportSize (contentsSize);
	Ui_.WebView_->page ()->mainFrame ()->render (&painter, clip);
	Ui_.WebView_->page ()->setViewportSize (oldSize);

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
				tr ("Error"),
				tr ("Could not open %1 for write")
					.arg (filename));
		return;
	}

	if (!file.write (dia->Save ()))
	{
		QMessageBox::critical (this,
				tr ("Error"),
				tr ("Could not write screenshot to %1")
					.arg (filename));
		return;
	}
}

void BrowserWidget::handleViewSources ()
{
	QString html = Ui_.WebView_->page ()->mainFrame ()->toHtml ();
	SourceViewer *viewer = new SourceViewer (this);
	viewer->setAttribute (Qt::WA_DeleteOnClose);
	viewer->SetHtml (html);
	viewer->show ();
}

void BrowserWidget::handleNewTab ()
{
	Core::Instance ().NewURL ("", true);
}

void BrowserWidget::focusLineEdit ()
{
	Ui_.URLEdit_->setFocus (Qt::OtherFocusReason);
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

	const int previewWidth = 200;
	if (!Ui_.WebView_->size ().isValid ())
		return;

	QSize contentsSize = Ui_.WebView_->page ()->mainFrame ()->contentsSize ();
	if (contentsSize.width () < 800)
		contentsSize.setWidth (800);
	int maxHeight = 0.8 * QApplication::desktop ()->screenGeometry (this).height () * static_cast<double> (contentsSize.width ()) / 200.0;
	contentsSize.setHeight (std::min (contentsSize.height (), 3000));
	QPoint scroll = Ui_.WebView_->page ()->mainFrame ()->scrollPosition ();
	QSize oldSize = Ui_.WebView_->page ()->viewportSize ();
	QRegion clip (0, 0, contentsSize.width (), contentsSize.height ());

	QPixmap pixmap (contentsSize);
	if (pixmap.isNull ())
		return;
	QPainter painter (&pixmap);
	Ui_.WebView_->page ()->setViewportSize (contentsSize);
	Ui_.WebView_->page ()->mainFrame ()->render (&painter, clip);
	Ui_.WebView_->page ()->setViewportSize (oldSize);
	Ui_.WebView_->page ()->mainFrame ()->setScrollPosition (scroll);
	painter.end ();

	QLabel *widget = new QLabel;

	if (pixmap.height () > 3000)
		pixmap = pixmap.copy (0, 0, pixmap.width (), 3000);

	pixmap = pixmap.scaledToWidth (previewWidth, Qt::SmoothTransformation);
	maxHeight = 0.8 * QApplication::desktop ()->screenGeometry (this).height ();
	if (pixmap.height () > maxHeight)
		pixmap = pixmap.copy (0, 0, 200, maxHeight);
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
}

void BrowserWidget::setupLoading ()
{
	Loading_.reset (new QMovie (":/resources/images/loading.gif"));
	Loading_->setBackgroundColor (QApplication::palette ().color (QPalette::Window));
	Loading_->start ();
	connect (Loading_.get (),
			SIGNAL (frameChanged (int)),
			this,
			SLOT (handleIconChanged ()));
}

void BrowserWidget::handleEntityAction ()
{
	emit gotEntity (qobject_cast<QAction*> (sender ())->data ().value<LeechCraft::DownloadEntity> ());
}

void BrowserWidget::handleLoadFinished ()
{
	if (HtmlMode_)
		return;

	ToolBar_->removeAction (ExternalLinks_->menuAction ());
	ExternalLinks_->clear ();

	QXmlStreamReader xml (Ui_.WebView_->page ()->mainFrame ()->toHtml ());
	bool inserted = false;
	while (!xml.atEnd ())
	{
		QXmlStreamReader::TokenType token = xml.readNext ();
		if (token == QXmlStreamReader::EndElement &&
				xml.name () == "head")
			break;
		else if (token != QXmlStreamReader::StartElement)
			continue;

		if (xml.name () != "link")
			continue;

		QXmlStreamAttributes attributes = xml.attributes ();
		if (attributes.value ("type") == "")
			continue;

		if (attributes.value ("rel") != "alternate" &&
				attributes.value ("rel") != "search")
			continue;

		LeechCraft::DownloadEntity e;
		e.Entity_ = attributes.value ("title").toString ().toUtf8 ();
		
		if (e.Entity_.isEmpty ())
			continue;

		e.Mime_ = attributes.value ("type").toString ();
		QString hrefUrl (attributes.value ("href").toString ());
		if (hrefUrl.indexOf ("://") < 0)
		{
			QUrl originalUrl = Ui_.WebView_->page ()->mainFrame ()->url ();
			originalUrl.setQueryItems (QList<QPair<QString, QString> > ());
			if (hrefUrl.size () &&
					hrefUrl.at (0) == '/')
				originalUrl.setPath (hrefUrl);
			else
			{
				QString originalPath = originalUrl.path ();
				if (!originalPath.endsWith ('/'))
				{
					int slashIndex = originalPath.lastIndexOf ('/');
					originalPath = originalPath.left (slashIndex + 1);
				}
				originalPath += hrefUrl;
				originalUrl.setPath (originalPath);
			}
			hrefUrl = originalUrl.toString ();
		}
		e.Location_ = hrefUrl;
		e.Parameters_ = LeechCraft::FromUserInitiated |
			LeechCraft::IsntDownloaded;

		bool ch = false;
		emit couldHandle (e, &ch);
		if (ch)
		{
			QString mime = e.Mime_;
			mime.replace ('/', '_');
			QAction *act = ExternalLinks_->
				addAction (QIcon (QString (":/resources/images/%1.png")
						.arg (mime)),
					QTextCodec::codecForName ("UTF-8")->toUnicode (e.Entity_),
					this,
					SLOT (handleEntityAction ()));
			act->setData (QVariant::fromValue<LeechCraft::DownloadEntity> (e));
			if (!inserted)
			{
				ToolBar_->insertAction (NewTab_, ExternalLinks_->menuAction ());
				inserted = true;
			}
		}
	}
}

void BrowserWidget::handleLoadProgress (int p)
{
	Ui_.Progress_->setValue (p);
	Ui_.Progress_->setVisible (p != 100);
}

