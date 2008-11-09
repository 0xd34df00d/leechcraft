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
#include <QtDebug>
#include "browserwidget.h"
#include "customwebview.h"
#include "addtofavoritesdialog.h"
#include "xmlsettingsmanager.h"
#include "customcookiejar.h"

Core::Core ()
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
	SaveCookies ();

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

void Core::SaveCookies () const
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

void Core::favoriteTagsUpdated (const QStringList& tags)
{
	XmlSettingsManager::Instance ()->setProperty ("FavoriteTags", tags);
}

