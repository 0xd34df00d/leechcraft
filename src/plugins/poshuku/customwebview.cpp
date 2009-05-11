#include "customwebview.h"
#include <QWebFrame>
#include <QMouseEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QtDebug>
#include "core.h"
#include "customwebpage.h"
#include "browserwidget.h"

CustomWebView::CustomWebView (QWidget *parent)
: QWebView (parent)
{
	Zooms_ << 0.3
		<< 0.5
		<< 0.67
		<< 0.8
		<< 0.9
		<< 1
		<< 1.1
		<< 1.2
		<< 1.33
		<< 1.5
		<< 1.7
		<< 2
		<< 2.4
		<< 3;

	CustomWebPage *page = new CustomWebPage (this);
	setPage (page);

	connect (this,
			SIGNAL (urlChanged (const QUrl&)),
			this,
			SLOT (remakeURL (const QUrl&)));

	connect (page,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
	connect (page,
			SIGNAL (loadingURL (const QUrl&)),
			this,
			SLOT (remakeURL (const QUrl&)));
	connect (page,
			SIGNAL (printRequested (QWebFrame*)),
			this,
			SIGNAL (printRequested (QWebFrame*)));
	connect (page,
			SIGNAL (windowCloseRequested ()),
			this,
			SIGNAL (closeRequested ()));
	connect (page,
			SIGNAL (storeFormData (const PageFormsData_t&)),
			this,
			SIGNAL (storeFormData (const PageFormsData_t&)));
}

CustomWebView::~CustomWebView ()
{
}

void CustomWebView::Load (const QString& string, QString title)
{
	Load (Core::Instance ().MakeURL (string), title);
}

void CustomWebView::Load (const QUrl& url, QString title)
{
	if (url.scheme () == "javascript")
	{
		QVariant result = page ()->mainFrame ()->
			evaluateJavaScript (url.toString ().mid (11));
		if (result.canConvert (QVariant::String))
			setHtml (result.toString ());
		return;
	}
	if (title.isEmpty ())
		title = tr ("Loading...");
	emit titleChanged (title);
	load (url);
}

void CustomWebView::Load (const QNetworkRequest& req,
		QNetworkAccessManager::Operation op, const QByteArray& ba)
{
	emit titleChanged (tr ("Loading..."));
	QWebView::load (req, op, ba);
}

QWebView* CustomWebView::createWindow (QWebPage::WebWindowType type)
{
	if (type == QWebPage::WebModalDialog)
	{
		// We don't need to register it in the Core, so construct
		// directly.
		BrowserWidget *widget = new BrowserWidget (this);
		widget->InitShortcuts ();
		widget->setWindowFlags (Qt::Dialog);
		widget->setAttribute (Qt::WA_DeleteOnClose);
		connect (widget,
				SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
				&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
		connect (widget,
				SIGNAL (titleChanged (const QString&)),
				widget,
				SLOT (setWindowTitle (const QString&)));
		return widget->GetView ();
	}
	else
		return Core::Instance ().MakeWebView ();
}

void CustomWebView::mousePressEvent (QMouseEvent *e)
{
	qobject_cast<CustomWebPage*> (page ())->SetButtons (e->buttons ());
	qobject_cast<CustomWebPage*> (page ())->SetModifiers (e->modifiers ());
	QWebView::mousePressEvent (e);
}

void CustomWebView::wheelEvent (QWheelEvent *e)
{
	if (e->modifiers () & Qt::ControlModifier)
	{
		int degrees = e->delta () / 8;
		qreal delta = static_cast<qreal> (degrees) / 150;
		setZoomFactor (zoomFactor () + delta);
		e->accept ();
	}
	else
		QWebView::wheelEvent (e);
}

void CustomWebView::contextMenuEvent (QContextMenuEvent *e)
{
	std::auto_ptr<QMenu> menu (new QMenu (this));
	QWebHitTestResult r = page ()->
		mainFrame ()->hitTestContent (e->pos ());

	if (!r.linkUrl ().isEmpty ())
	{
		menu->addAction (tr ("Open &here"),
				this, SLOT (openLinkHere ()))->setData (r.linkUrl ());
		menu->addAction (tr ("Open in new &tab"),
				this, SLOT (openLinkInNewTab ()));
		menu->addSeparator ();
		menu->addAction (tr ("&Save link..."),
				this, SLOT (saveLink ()));

		QList<QVariant> datalist;
		datalist << r.linkUrl ()
			<< r.linkText ();
		menu->addAction (tr ("&Bookmark link..."),
				this, SLOT (bookmarkLink ()))->setData (datalist);

		menu->addSeparator ();
		if (!page ()->selectedText ().isEmpty ())
			menu->addAction (pageAction (QWebPage::Copy));
		menu->addAction (tr ("&Copy link"),
				this, SLOT (copyLink ()));
	}

	if (!r.imageUrl ().isEmpty ())
	{
		if (!menu->isEmpty ())
			menu->addSeparator ();
		menu->addAction (tr ("Open image here"),
				this, SLOT (openImageHere ()))->setData (r.imageUrl ());
		menu->addAction (tr ("Open image in new tab"),
				this, SLOT (openImageInNewTab ()));
		menu->addSeparator ();
		menu->addAction (tr ("Save image..."),
				this, SLOT (saveImage ()));
		menu->addAction (tr ("Copy image"),
				this, SLOT (copyImage ()));
		menu->addAction (tr ("Copy image location"),
				this, SLOT (copyImageLocation ()))->setData (r.imageUrl ());
	}

	if (menu->isEmpty ())
		menu.reset (page ()->createStandardContextMenu ());

	if (!menu->isEmpty ())
	{
		menu->exec (mapToGlobal (e->pos ()));
		return;
	}

	QWebView::contextMenuEvent (e);
}

int CustomWebView::LevelForZoom (qreal zoom)
{
	int i = Zooms_.indexOf (zoom);

	if (i >= 0)
		return i;

	for (i = 0; i < Zooms_.size (); ++i)
		if (zoom <= Zooms_ [i])
			break;

	if (i == Zooms_.size ())
		return i - 1;

	if (i == 0)
		return i;

	if (zoom - Zooms_ [i - 1] > Zooms_ [i] - zoom)
		return i;
	else
		return i - 1;
}

void CustomWebView::zoomIn ()
{
	int i = LevelForZoom (zoomFactor ());

	if (i < Zooms_.size () - 1)
		setZoomFactor (Zooms_ [i + 1]);
}

void CustomWebView::zoomOut ()
{
	int i = LevelForZoom (zoomFactor ());

	if (i > 0)
		setZoomFactor (Zooms_ [i - 1]);
}

void CustomWebView::zoomReset ()
{
	setZoomFactor (1);
}

void CustomWebView::remakeURL (const QUrl& url)
{
	emit urlChanged (url.toString ());
}

void CustomWebView::openLinkHere ()
{
	Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
}

void CustomWebView::openLinkInNewTab ()
{
	pageAction (QWebPage::OpenLinkInNewWindow)->trigger ();
}

void CustomWebView::saveLink ()
{
	pageAction (QWebPage::DownloadLinkToDisk)->trigger ();
}

void CustomWebView::bookmarkLink ()
{
	QList<QVariant> list = qobject_cast<QAction*> (sender ())->data ().toList ();
	emit addToFavorites (list.at (1).toString (),
			list.at (0).toUrl ().toString ());
}

void CustomWebView::copyLink ()
{
	pageAction (QWebPage::CopyLinkToClipboard)->trigger ();
}

void CustomWebView::openImageHere ()
{
	Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
}

void CustomWebView::openImageInNewTab ()
{
	pageAction (QWebPage::OpenImageInNewWindow)->trigger ();
}

void CustomWebView::saveImage ()
{
	pageAction (QWebPage::DownloadImageToDisk)->trigger ();
}

void CustomWebView::copyImage ()
{
	pageAction (QWebPage::CopyImageToClipboard)->trigger ();
}

void CustomWebView::copyImageLocation ()
{
	QString url = qobject_cast<QAction*> (sender ())->data ().toUrl ().toString ();
	QClipboard *cb = QApplication::clipboard ();
	cb->setText (url, QClipboard::Clipboard);
	cb->setText (url, QClipboard::Selection);
}

