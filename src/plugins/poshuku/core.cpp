#include "core.h"
#include <algorithm>
#include <memory>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QIcon>
#include <QFile>
#include <QNetworkCookieJar>
#include <QDir>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QtDebug>
#include "browserwidget.h"
#include "customwebview.h"
#include "addtofavoritesdialog.h"
#include "xmlsettingsmanager.h"
#include "customcookiejar.h"
#include "authenticationdialog.h"

Core::Core ()
: CookieSaveTimer_ (new QTimer ())
{
	NetworkAccessManager_.reset (new QNetworkAccessManager (this));
	QFile file (QDir::homePath () +
			"/.leechcraft/poshuku/cookies.txt");
	if (file.open (QIODevice::ReadOnly))
	{
		CustomCookieJar *jar = new CustomCookieJar (this);
		jar->Load (file.readAll ());
		NetworkAccessManager_->setCookieJar (jar);
	}

	connect (NetworkAccessManager_.get (),
			SIGNAL (authenticationRequired (QNetworkReply*,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (QNetworkReply*, QAuthenticator*)));
	connect (NetworkAccessManager_.get (),
			SIGNAL (proxyAuthenticationRequired (QNetworkReply*,
					QAuthenticator*)),
			this,
			SLOT (handleProxyAuthentication (QNetworkReply*,
					QAuthenticator*)));

	connect (CookieSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);

	FavoritesModel_.reset (new FavoritesModel (this));

	FavoriteTagsCompletionModel_.reset (new TagsCompletionModel (this));
	FavoriteTagsCompletionModel_->
		UpdateTags (XmlSettingsManager::Instance ()->
				Property ("FavoriteTags", tr ("untagged")).toStringList ());
	connect (FavoriteTagsCompletionModel_.get (),
			SIGNAL (tagsUpdated (const QStringList&)),
			this,
			SLOT (favoriteTagsUpdated (const QStringList&)));
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	CookieSaveTimer_.reset ();

	saveCookies ();

	NetworkAccessManager_.reset ();
	FavoritesModel_.reset ();
	FavoriteTagsCompletionModel_.reset ();
}

bool Core::IsValidURL (const QString& url) const
{
	return QUrl (url).isValid ();
}

BrowserWidget* Core::NewURL (const QString& url)
{
	BrowserWidget *widget = new BrowserWidget;
	widget->GetView ()->page ()->
		setNetworkAccessManager (GetNetworkAccessManager ());
	widget->SetURL (QUrl (url));

	connect (widget,
			SIGNAL (titleChanged (const QString&)),
			this,
			SLOT (handleTitleChanged (const QString&)));
	connect (widget,
			SIGNAL (iconChanged (const QIcon&)),
			this,
			SLOT (handleIconChanged (const QIcon&)));
	connect (widget,
			SIGNAL (needToClose ()),
			this,
			SLOT (handleNeedToClose ()));
	connect (widget,
			SIGNAL (addToFavorites (const QString&, const QString&)),
			this,
			SLOT (handleAddToFavorites (const QString&, const QString&)));

	Widgets_.push_back (widget);

	emit addNewTab (tr ("Loading..."), widget);
	return widget;
}

CustomWebView* Core::MakeWebView ()
{
	return NewURL ("")->GetView ();
}

FavoritesModel* Core::GetFavoritesModel () const
{
	return FavoritesModel_.get ();
}

TagsCompletionModel* Core::GetFavoritesTagsCompletionModel () const
{
	return FavoriteTagsCompletionModel_.get ();
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return NetworkAccessManager_.get ();
}

void Core::saveCookies () const
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	if (!dir.exists ("poshuku") &&
			!dir.mkdir ("poshuku"))
	{
		emit error (tr ("Could not create Poshuku directory."));
		return;
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/poshuku/cookies.txt");
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		emit error (tr ("Could not save cookies, error opening cookie file."));
	else
		file.write (static_cast<CustomCookieJar*> (NetworkAccessManager_->cookieJar ())->Save ());
}

void Core::handleTitleChanged (const QString& newTitle)
{
	emit changeTabName (dynamic_cast<QWidget*> (sender ()), newTitle);
}

void Core::handleIconChanged (const QIcon& newIcon)
{
	emit changeTabIcon (dynamic_cast<QWidget*> (sender ()), newIcon);
}

void Core::handleNeedToClose ()
{
	BrowserWidget *w = dynamic_cast<BrowserWidget*> (sender ());
	emit removeTab (w);

	std::remove (Widgets_.begin (), Widgets_.end (), w);
	w->deleteLater ();
}

void Core::handleAddToFavorites (const QString& title, const QString& url)
{
	std::auto_ptr<AddToFavoritesDialog> dia (new AddToFavoritesDialog (title,
				url,
				GetFavoritesTagsCompletionModel (),
				qApp->activeWindow ()));
	if (dia->exec () == QDialog::Rejected)
		return;

	FavoritesModel_->AddItem (dia->GetTitle (), url, dia->GetTags ());
	FavoriteTagsCompletionModel_->UpdateTags (dia->GetTags ());
}

void Core::handleAuthentication (QNetworkReply *reply, QAuthenticator *authen)
{
	QString msg = tr ("The URL<br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (reply->url ().toString ())
		.arg (authen->realm ());
	msg = msg.left (200);
	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				authen->user (),
				authen->password (),
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	authen->setUser (dia->GetLogin ());
	authen->setPassword (dia->GetPassword ());
}

void Core::handleProxyAuthentication (QNetworkReply *reply, QAuthenticator *authen)
{
	QString msg = tr ("Proxy for URL<br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (reply->url ().toString ())
		.arg (authen->realm ());
	msg = msg.left (200);
	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				authen->user (),
				authen->password (),
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	authen->setUser (dia->GetLogin ());
	authen->setPassword (dia->GetPassword ());
}

void Core::favoriteTagsUpdated (const QStringList& tags)
{
	XmlSettingsManager::Instance ()->setProperty ("FavoriteTags", tags);
}

