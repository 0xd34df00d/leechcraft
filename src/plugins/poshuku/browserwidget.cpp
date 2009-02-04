#include "browserwidget.h"
#include <QKeyEvent>
#include <QtDebug>
#include <QToolBar>
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

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);

	QToolBar *bar = new QToolBar ();
	
	QAction *back = Ui_.WebView_->pageAction (QWebPage::Back);
	back->setParent (this);
	back->setProperty ("ActionIcon", "poshuku_back");

	QAction *forward = Ui_.WebView_->pageAction (QWebPage::Forward);
	forward->setParent (this);
	forward->setProperty ("ActionIcon", "poshuku_forward");

	QAction *reload = Ui_.WebView_->pageAction (QWebPage::Reload);
	reload->setParent (this);
	reload->setShortcut (Qt::Key_F5);
	reload->setProperty ("ActionIcon", "poshuku_reload");

	QAction *stop = Ui_.WebView_->pageAction (QWebPage::Stop);
	stop->setParent (this);
	stop->setShortcut (Qt::Key_Escape);
	stop->setProperty ("ActionIcon", "poshuku_stop");

	QAction *add2Favorites = new QAction (tr ("Add to favorites..."),
			this);
	add2Favorites->setProperty ("ActionIcon", "poshuku_addtofavorites");

	QAction *find = new QAction (tr ("Find..."),
			this);
	find->setShortcut (Qt::Key_F3);
	find->setProperty ("ActionIcon", "poshuku_find");

	QAction *print = new QAction (tr ("Print..."),
			this);
	print->setProperty ("ActionIcon", "poshuku_print");

	QAction *printPreview = new QAction (tr ("Print with preview..."),
			this);
	printPreview->setProperty ("ActionIcon", "poshuku_printpreview");

	QAction *screenSave = new QAction (tr ("Take page's screenshot"),
			this);
	screenSave->setProperty ("ActionIcon", "poshuku_takescreenshot");

	bar->addAction (back);
	bar->addAction (forward);
	bar->addAction (reload);
	bar->addAction (stop);
	bar->addAction (find);
	bar->addAction (add2Favorites);
	bar->addAction (print);
	bar->addAction (printPreview);
	bar->addAction (screenSave);

	QWidgetAction *addressBar = new QWidgetAction (this);
	addressBar->setDefaultWidget (Ui_.URLEdit_);
	bar->addAction (addressBar);

	static_cast<QVBoxLayout*> (layout ())->insertWidget (0, bar);

	connect (add2Favorites,
			SIGNAL (triggered ()),
			this,
			SLOT (handleAdd2Favorites ()));
	connect (print,
			SIGNAL (triggered ()),
			this,
			SLOT (handlePrinting ()));
	connect (printPreview,
			SIGNAL (triggered ()),
			this,
			SLOT (handlePrintingWithPreview ()));
	connect (find,
			SIGNAL (triggered ()),
			this,
			SLOT (handleFind ()));
	connect (screenSave,
			SIGNAL (triggered ()),
			this,
			SLOT (handleScreenSave ()));

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
			SIGNAL (gotEntity (const QByteArray&)),
			this,
			SIGNAL (gotEntity (const QByteArray&)));
	connect (Ui_.WebView_->page (),
			SIGNAL (linkHovered (const QString&,
					const QString&,
					const QString&)),
			this,
			SLOT (handleStatusBarMessage (const QString&)));

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

CustomWebView* BrowserWidget::GetView () const
{
	return Ui_.WebView_;
}

void BrowserWidget::SetURL (const QUrl& url)
{
	if (!url.isEmpty ())
		Ui_.WebView_->Load (url);
}

void BrowserWidget::keyReleaseEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_T &&
			e->modifiers () & Qt::ControlModifier)
		Core::Instance ().NewURL ("", true);
	else
		QWidget::keyReleaseEvent (e);
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

	QString url = QUrl (Ui_.URLEdit_->text ()).toString ();
	if (!Core::Instance ().IsValidURL (url))
		return;

	Ui_.WebView_->Load (QUrl (url));
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

void BrowserWidget::focusLineEdit ()
{
	Ui_.URLEdit_->setFocus (Qt::OtherFocusReason);
}

