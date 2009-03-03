#include "browserwidget.h"
#include <QKeyEvent>
#include <QtDebug>
#include <QToolBar>
#include <QMenu>
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
#include "core.h"
#include "historymodel.h"
#include "finddialog.h"
#include "screenshotsavedialog.h"
#include "xmlsettingsmanager.h"
#include "speeddialprovider.h"
#include "sourceviewer.h"

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);

	Ui_.WebView_->pageAction (QWebPage::Cut)->setProperty ("ActionIcon", "poshuku_cut");
	Ui_.WebView_->pageAction (QWebPage::Cut)->setShortcut (tr ("Ctrl+X"));
	Ui_.WebView_->pageAction (QWebPage::Copy)->setProperty ("ActionIcon", "poshuku_copy");
	Ui_.WebView_->pageAction (QWebPage::Copy)->setShortcut (tr ("Ctrl+C"));
	Ui_.WebView_->pageAction (QWebPage::Paste)->setProperty ("ActionIcon", "poshuku_paste");
	Ui_.WebView_->pageAction (QWebPage::Paste)->setShortcut (tr ("Ctrl+V"));

	QToolBar *bar = new QToolBar (this);
	
	QAction *back = Ui_.WebView_->pageAction (QWebPage::Back);
	back->setParent (this);
	back->setProperty ("ActionIcon", "poshuku_back");
	back->setShortcut (Qt::ALT + Qt::Key_Left);

	QAction *forward = Ui_.WebView_->pageAction (QWebPage::Forward);
	forward->setParent (this);
	forward->setProperty ("ActionIcon", "poshuku_forward");
	forward->setShortcut (Qt::ALT + Qt::Key_Right);

	QAction *reload = Ui_.WebView_->pageAction (QWebPage::Reload);
	reload->setParent (this);
	reload->setShortcut (Qt::Key_F5);
	reload->setProperty ("ActionIcon", "poshuku_reload");

	QAction *stop = Ui_.WebView_->pageAction (QWebPage::Stop);
	stop->setParent (this);
	stop->setShortcut (Qt::Key_Escape);
	stop->setProperty ("ActionIcon", "poshuku_stop");

	QMenu *moreMenu = new QMenu (this);
	QAction *more = moreMenu->menuAction ();
	more->setText (tr ("More..."));
	more->setProperty ("ActionIcon", "poshuku_more");

	Add2Favorites_ = new QAction (tr ("Add to favorites..."),
			this);
	Add2Favorites_->setProperty ("ActionIcon", "poshuku_addtofavorites");
	Add2Favorites_->setShortcut (tr ("Ctrl+D"));
	Add2Favorites_->setEnabled (false);

	Find_ = new QAction (tr ("Find..."),
			this);
	Find_->setShortcut (tr ("Ctrl+F"));
	Find_->setProperty ("ActionIcon", "poshuku_find");
	Find_->setEnabled (false);

	Print_ = new QAction (tr ("Print..."),
			this);
	Print_->setProperty ("ActionIcon", "poshuku_print");
	Print_->setShortcut (tr ("Ctrl+P"));
	Print_->setEnabled (false);

	PrintPreview_ = new QAction (tr ("Print with preview..."),
			this);
	PrintPreview_->setProperty ("ActionIcon", "poshuku_printpreview");
	PrintPreview_->setShortcut (tr ("Ctrl+Shift+P"));
	PrintPreview_->setEnabled (false);

	ScreenSave_ = new QAction (tr ("Take page's screenshot..."),
			this);
	ScreenSave_->setProperty ("ActionIcon", "poshuku_takescreenshot");
	ScreenSave_->setShortcut (Qt::Key_F12);
	ScreenSave_->setEnabled (false);

	ViewSources_ = new QAction (tr ("View sources..."),
			this);
	ViewSources_->setProperty ("ActionIcon", "poshuku_viewsources");
	ViewSources_->setEnabled (false);

	NewTab_ = new QAction (tr ("Create new tab"),
			this);
	NewTab_->setProperty ("ActionIcon", "poshuku_newtab");
	NewTab_->setShortcut (tr ("Ctrl+T"));

	CloseTab_ = new QAction (tr ("Close this tab"),
			this);
	CloseTab_->setProperty ("ActionIcon", "poshuku_closetab");
	CloseTab_->setShortcut (tr ("Ctrl+W"));

	bar->addAction (back);
	bar->addAction (forward);
	bar->addAction (reload);
	bar->addAction (stop);
	bar->addAction (more);

	moreMenu->addAction (Find_);
	moreMenu->addAction (Add2Favorites_);
	moreMenu->addAction (Print_);
	moreMenu->addAction (PrintPreview_);
	moreMenu->addAction (ScreenSave_);
	moreMenu->addAction (ViewSources_);
	moreMenu->addSeparator ();
	RecentlyClosed_ = moreMenu->addMenu (tr ("Recently closed"));
	RecentlyClosed_->setEnabled (false);
	RecentlyClosed_->menuAction ()->setShortcut (tr ("Ctrl+Shift+T"));

	QWidgetAction *addressBar = new QWidgetAction (this);
	addressBar->setDefaultWidget (Ui_.URLEdit_);
	bar->addAction (addressBar);

	bar->addAction (NewTab_);
	bar->addAction (CloseTab_);

	static_cast<QVBoxLayout*> (layout ())->insertWidget (0, bar);

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
			Ui_.URLEdit_,
			SLOT (setValue (int)));
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
			SIGNAL (loadStarted ()),
			this,
			SLOT (enableActions ()));

	connect (&Core::Instance (),
			SIGNAL (newUnclose (QAction*)),
			this,
			SLOT (handleNewUnclose (QAction*)));

	QCompleter *completer = new QCompleter (this);
	completer->setModel (Core::Instance ().GetURLCompletionModel ());
	Ui_.URLEdit_->setCompleter (completer);
	connect (Ui_.URLEdit_,
			SIGNAL (textChanged (const QString&)),
			Core::Instance ().GetURLCompletionModel (),
			SLOT (setBase (const QString&)));

	QTimer::singleShot (100,
			this,
			SLOT (focusLineEdit ()));
}

BrowserWidget::~BrowserWidget ()
{
	Core::Instance ().Unregister (this);
}

void BrowserWidget::SetUnclosers (const QList<QAction*>& unclosers)
{
	RecentlyClosed_->addActions (unclosers);
	if (unclosers.size ())
	{
		RecentlyClosed_->setEnabled (true);
		RecentlyClosed_->setDefaultAction (unclosers.front ());
		connect (RecentlyClosed_->menuAction (),
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
		Ui_.WebView_->Load (url);
}

void BrowserWidget::Load (const QString& url)
{
	SetURL (Core::Instance ().MakeURL (url));
}

void BrowserWidget::SetHtml (const QString& html, const QString& base)
{
	Ui_.URLEdit_->clear ();
	Ui_.WebView_->setHtml (html, base);
}

QWidget* BrowserWidget::Widget ()
{
	return this;
}

void BrowserWidget::PrintImpl (bool preview)
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
				Ui_.WebView_,
				SLOT (print (QPrinter*)));

		if (prevDialog->exec () != QDialog::Accepted)
			return;
	}

	Ui_.WebView_->print (printer.get ());
}

void BrowserWidget::handleIconChanged ()
{
	emit iconChanged (Ui_.WebView_->icon ());
}

void BrowserWidget::handleStatusBarMessage (const QString& msg)
{
	emit statusBarChanged (msg);
}

void BrowserWidget::on_URLEdit__returnPressed ()
{
	if (Ui_.URLEdit_->IsCompleting ())
		return;

	Ui_.WebView_->Load (Ui_.URLEdit_->text ());
}

void BrowserWidget::handleAdd2Favorites ()
{
	emit addToFavorites (Ui_.WebView_->title (),
			Ui_.WebView_->url ().toString ());
}

void BrowserWidget::handleFind ()
{
	FindDialog *dialog = new FindDialog (this);
	
	connect (dialog,
			SIGNAL (next (const QString&, QWebPage::FindFlags)),
			this,
			SLOT (findText (const QString&, QWebPage::FindFlags)));

	dialog->show ();
}

void BrowserWidget::findText (const QString& text,
		QWebPage::FindFlags flags)
{
	bool found = Ui_.WebView_->page ()->findText (text, flags);
	static_cast<FindDialog*> (sender ())->SetSuccessful (found);
}

void BrowserWidget::handlePrinting ()
{
	PrintImpl (false);
}

void BrowserWidget::handlePrintingWithPreview ()
{
	PrintImpl (true);
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
	disconnect (RecentlyClosed_->menuAction (),
			SIGNAL (triggered ()),
			0,
			0);
	connect (RecentlyClosed_->menuAction (),
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
		disconnect (RecentlyClosed_->menuAction (),
				SIGNAL (triggered ()),
				0,
				0);
		connect (RecentlyClosed_->menuAction (),
				SIGNAL (triggered ()),
				RecentlyClosed_->actions ().front (),
				SLOT (trigger ()));
		RecentlyClosed_->setDefaultAction (RecentlyClosed_->actions ().front ());
	}
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

