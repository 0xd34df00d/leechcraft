#include "browserwidget.h"
#include <QDebug>
#include <QToolBar>
#include "core.h"
#include "xmlsettingsmanager.h"

BrowserWidget::BrowserWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);

	connect (page ()->networkAccessManager (),
			SIGNAL (sslErrors (QNetworkReply*, const QList<QSslError>&)),
			&Core::Instance (),
			SLOT (handleSslError (QNetworkReply*)));

	connect (Ui_.ItemView_,
			SIGNAL (loadStarted ()),
			this,
			SLOT (loadStarted ()));
	connect (Ui_.ItemView_,
			SIGNAL (loadFinished (bool)),
			this,
			SLOT (loadFinished (bool)));
	connect (Ui_.ItemView_,
			SIGNAL (loadProgress (int)),
			this,
			SLOT (loadProgress (int)));

	QToolBar *bar = new QToolBar ();

	QAction *back = Ui_.ItemView_->pageAction (QWebPage::Back);
	back->setParent (this);
	back->setProperty ("ActionIcon", "aggregator_larrow");

	QAction *forward = Ui_.ItemView_->pageAction (QWebPage::Forward);
	forward->setParent (this);
	forward->setProperty ("ActionIcon", "aggregator_rarrow");

	QAction *reload = Ui_.ItemView_->pageAction (QWebPage::Reload);
	reload->setParent (this);
	reload->setShortcut (Qt::Key_F5);
	reload->setProperty ("ActionIcon", "aggregator_refresh");

	QAction *stop = Ui_.ItemView_->pageAction (QWebPage::Stop);
	stop->setParent (this);
	stop->setShortcut (Qt::Key_Escape);
	stop->setProperty ("ActionIcon", "aggregator_stop");

	bar->addAction (back);
	bar->addAction (forward);
	bar->addAction (reload);
	bar->addAction (stop);

	Ui_.ToolLayout_->insertWidget (0, bar);

	QList<QByteArray> viewerSettings;
	viewerSettings << "StandardFont"
		<< "FixedFont"
		<< "SerifFont"
		<< "SansSerifFont"
		<< "CursiveFont"
		<< "FantasyFont"
		<< "MinimumFontSize"
		<< "DefaultFontSize"
		<< "DefaultFixedFontSize"
		<< "AutoLoadImages"
		<< "AllowJavaScript";
	XmlSettingsManager::Instance ()->RegisterObject (viewerSettings,
			this, "viewerSettingsChanged");

	viewerSettingsChanged ();
}

BrowserWidget::~BrowserWidget ()
{
}

void BrowserWidget::setHtml (const QString& html, const QString& base)
{
	Ui_.ItemView_->setHtml (html, base);
}

QWebPage* BrowserWidget::page () const
{
	return Ui_.ItemView_->page ();
}

void BrowserWidget::loadStarted ()
{
	Ui_.PageLoadProgressBar_->setFormat (tr ("Loading..."));
	Ui_.PageLoadProgressBar_->setValue (100);
}

void BrowserWidget::loadFinished (bool ok)
{
	QString title = Ui_.ItemView_->title ();
	if (title.isEmpty ())
		title = tr ("Loaded");
	QString text = ok ? title : title + tr (" (with errors)");
	Ui_.PageLoadProgressBar_->setFormat (text);
	Ui_.PageLoadProgressBar_->setValue (100);
}

void BrowserWidget::loadProgress (int progress)
{
	Ui_.PageLoadProgressBar_->setFormat (tr ("Loading (\%p)"));
	Ui_.PageLoadProgressBar_->setValue (progress);
}

QWebSettings *BrowserWidget::settings () const
{
	return Ui_.ItemView_->settings ();
}

void BrowserWidget::viewerSettingsChanged ()
{
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::StandardFont,
			XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FixedFont,
			XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SerifFont,
			XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::SansSerifFont,
			XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::CursiveFont,
			XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
	Ui_.ItemView_->settings ()->setFontFamily (QWebSettings::FantasyFont,
			XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());

	Ui_.ItemView_->settings ()->setFontSize (QWebSettings::MinimumFontSize,
			XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
	Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
	Ui_.ItemView_->settings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
	Ui_.ItemView_->settings ()->setAttribute (QWebSettings::AutoLoadImages,
			XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
	Ui_.ItemView_->settings ()->setAttribute (QWebSettings::JavascriptEnabled,
			XmlSettingsManager::Instance ()->property ("AllowJavaScript").toBool ());
}

