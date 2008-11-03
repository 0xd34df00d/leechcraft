#include "browserwidget.h"
#include <QKeyEvent>
#include <QtDebug>

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);

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
			Ui_.LoadProgress_,
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
	if (e->key () == Qt::Key_W &&
			e->modifiers () & Qt::ControlModifier)
	{
		emit needToClose ();
		e->accept ();
	}
}

void BrowserWidget::handleIconChanged ()
{
	qDebug () << Q_FUNC_INFO << Ui_.WebView_->icon ();
	emit iconChanged (Ui_.WebView_->icon ());
}

void BrowserWidget::handleStatusBarMessage (const QString& str)
{
	Ui_.LoadProgress_->setFormat (str + " (%p)");
}

