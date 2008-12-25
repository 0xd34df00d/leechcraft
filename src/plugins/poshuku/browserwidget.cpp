#include "browserwidget.h"
#include <QKeyEvent>
#include <QtDebug>
#include <QToolBar>
#include <QCompleter>
#include <QWidgetAction>
#include "core.h"
#include "historymodel.h"

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
	reload->setProperty ("ActionIcon", "poshuku_reload");

	QAction *stop = Ui_.WebView_->pageAction (QWebPage::Stop);
	stop->setParent (this);
	stop->setProperty ("ActionIcon", "poshuku_stop");

	QAction *add2Favorites = new QAction (tr ("Add to favorites..."),
			this);
	add2Favorites->setProperty ("ActionIcon", "poshuku_addtofavorites");

	bar->addAction (back);
	bar->addAction (forward);
	bar->addAction (reload);
	bar->addAction (stop);
	bar->addAction (add2Favorites);

	QWidgetAction *addressBar = new QWidgetAction (this);
	addressBar->setDefaultWidget (Ui_.URLEdit_);
	bar->addAction (addressBar);

	static_cast<QVBoxLayout*> (layout ())->insertWidget (0, bar);

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
	connect (Ui_.WebView_->page (),
			SIGNAL (linkHovered (const QString&,
					const QString&,
					const QString&)),
			this,
			SLOT (handleStatusBarMessage (const QString&)));

	QCompleter *completer = new QCompleter (this);
	completer->setModel (Core::Instance ().GetURLCompletionModel ());
	Ui_.URLEdit_->setCompleter (completer);
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

void BrowserWidget::keyReleaseEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_T &&
			e->modifiers () & Qt::ControlModifier)
		Core::Instance ().NewURL ("");
	else
		QWidget::keyReleaseEvent (e);
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

