#include "browserwidget.h"
#include <QKeyEvent>
#include <QtDebug>
#include <QToolBar>
#include "core.h"

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);

	QToolBar *bar = new QToolBar ();
	
	QAction *back = new QAction (tr ("Back"),
			this);
	back->setProperty ("ActionIcon", "poshuku_back");

	QAction *forward = new QAction (tr ("Forward"),
			this);
	forward->setProperty ("ActionIcon", "poshuku_forward");

	QAction *reload = new QAction (tr ("Reload"),
			this);
	reload->setProperty ("ActionIcon", "poshuku_reload");

	QAction *stop = new QAction (tr ("Stop"),
			this);
	stop->setProperty ("ActionIcon", "poshuku_stop");

	QAction *add2Favorites = new QAction (tr ("Add to favorites..."),
			this);
	add2Favorites->setProperty ("ActionIcon", "poshuku_addtofavorites");

	bar->addAction (back);
	bar->addAction (forward);
	bar->addAction (reload);
	bar->addAction (stop);
	bar->addAction (add2Favorites);

	static_cast<QVBoxLayout*> (layout ())->insertWidget (0, bar);

	connect (back,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (back ()));
	connect (forward,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (forward ()));
	connect (reload,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (reload ()));
	connect (stop,
			SIGNAL (triggered ()),
			Ui_.WebView_,
			SLOT (stop ()));
	connect (add2Favorites,
			SIGNAL (triggered ()),
			this,
			SLOT (handleAdd2Favorites ()));

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
	Ui_.WebView_->load (url);
}

void BrowserWidget::handleIconChanged ()
{
	qDebug () << Q_FUNC_INFO << Ui_.WebView_->icon ();
	emit iconChanged (Ui_.WebView_->icon ());
}

void BrowserWidget::handleStatusBarMessage (const QString&)
{
//	Ui_.LoadProgress_->setFormat (str + " (%p)");
}

void BrowserWidget::on_URLEdit__returnPressed ()
{
	QString url = Ui_.URLEdit_->text ();
	if (!Core::Instance ().IsValidURL (url))
		return;

	Ui_.WebView_->load (QUrl (url));
}

void BrowserWidget::handleAdd2Favorites ()
{
	emit addToFavorites (Ui_.WebView_->title (),
			Ui_.WebView_->url ().toString ());
}

