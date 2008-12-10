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
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QFileDialog>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include <interfaces/interfaces.h>
#include "browserwidget.h"
#include "customwebview.h"
#include "addtofavoritesdialog.h"
#include "xmlsettingsmanager.h"
#include "customcookiejar.h"
#include "authenticationdialog.h"
#include "sslerrorsdialog.h"
#include "restoresessiondialog.h"
#include "sqlstoragebackend.h"

Core::Core ()
: CookieSaveTimer_ (new QTimer ())
, SaveSessionScheduled_ (false)
{
	QDir dir = QDir::home ();
	if (!dir.cd (".leechcraft/poshuku") &&
			!dir.mkpath (".leechcraft/poshuku"))
	{
		qCritical () << Q_FUNC_INFO << "could not create neccessary "
			"directories for Poshuku";
		return;
	}

	StorageBackend_.reset (new SQLStorageBackend);
	StorageBackend_->Prepare ();

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
			SLOT (handleAuthentication (QNetworkReply*,
					QAuthenticator*)));
	connect (NetworkAccessManager_.get (),
			SIGNAL (proxyAuthenticationRequired (const QNetworkProxy&,
					QAuthenticator*)),
			this,
			SLOT (handleProxyAuthentication (const QNetworkProxy&,
					QAuthenticator*)));
	connect (NetworkAccessManager_.get (),
			SIGNAL (sslErrors (QNetworkReply*,
					const QList<QSslError>&)),
			this,
			SLOT (handleSslErrors (QNetworkReply*,
					const QList<QSslError>&)));

	connect (CookieSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);

	HistoryModel_.reset (new HistoryModel (this));
	connect (StorageBackend_.get (),
			SIGNAL (added (const HistoryModel::HistoryItem&)),
			HistoryModel_.get (),
			SLOT (handleItemAdded (const HistoryModel::HistoryItem&)));

	FavoritesModel_.reset (new FavoritesModel (this));
	connect (StorageBackend_.get (),
			SIGNAL (added (const FavoritesModel::FavoritesItem&)),
			FavoritesModel_.get (),
			SLOT (handleItemAdded (const FavoritesModel::FavoritesItem&)));
	connect (StorageBackend_.get (),
			SIGNAL (updated (const FavoritesModel::FavoritesItem&)),
			FavoritesModel_.get (),
			SLOT (handleItemUpdated (const FavoritesModel::FavoritesItem&)));
	connect (StorageBackend_.get (),
			SIGNAL (removed (const FavoritesModel::FavoritesItem&)),
			FavoritesModel_.get (),
			SLOT (handleItemRemoved (const FavoritesModel::FavoritesItem&)));

	FavoriteTagsCompletionModel_.reset (new TagsCompletionModel (this));
	FavoriteTagsCompletionModel_->
		UpdateTags (XmlSettingsManager::Instance ()->
				Property ("FavoriteTags", tr ("untagged")).toStringList ());
	connect (FavoriteTagsCompletionModel_.get (),
			SIGNAL (tagsUpdated (const QStringList&)),
			this,
			SLOT (favoriteTagsUpdated (const QStringList&)));

	QTimer::singleShot (200, this, SLOT (postConstruct ()));
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
	HistoryModel_.reset ();
	FavoritesModel_.reset ();
	FavoriteTagsCompletionModel_.reset ();

	XmlSettingsManager::Instance ()->setProperty ("CleanShutdown", true);
	XmlSettingsManager::Instance ()->Release ();
}

void Core::SetProvider (QObject *object, const QString& feature)
{
	Providers_ [feature] = object;

	if (qobject_cast<IDownload*> (object))
		Downloaders_ << object;
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
	connect (widget,
			SIGNAL (urlChanged (const QString&)),
			this,
			SLOT (handleURLChanged (const QString&)));

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

HistoryModel* Core::GetHistoryModel () const
{
	return HistoryModel_.get ();
}

TagsCompletionModel* Core::GetFavoritesTagsCompletionModel () const
{
	return FavoriteTagsCompletionModel_.get ();
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return NetworkAccessManager_.get ();
}

StorageBackend* Core::GetStorageBackend () const
{
	return StorageBackend_.get ();
}

void Core::Unregister (BrowserWidget *widget)
{
	widgets_t::iterator pos =
		std::find (Widgets_.begin (), Widgets_.end (), widget);
	if (pos == Widgets_.end ())
		return;

	Widgets_.erase (pos);
}

void Core::DoCommonAuth (const QString& msg, QAuthenticator *authen)
{
	QString realm = authen->realm ();

	QString suggestedUser = authen->user ();
	QString suggestedPassword = authen->password ();

	if (suggestedUser.isEmpty ())
		StorageBackend_->GetAuth (realm, suggestedUser, suggestedPassword);

	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				suggestedUser,
				suggestedPassword,
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	QString login = dia->GetLogin ();
	QString password = dia->GetPassword ();
	authen->setUser (login);
	authen->setPassword (password);

	if (dia->ShouldSave ())
		StorageBackend_->SetAuth (realm, login, password);
}

void Core::RestoreSession (bool ask)
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	int size = settings.beginReadArray ("Saved session");
	if (!size) ;
	else if (ask)
	{
		std::auto_ptr<RestoreSessionDialog> dia (new RestoreSessionDialog ());
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QString title = settings.value ("Title").toString ();
			QString url = settings.value ("URL").toString ();
			dia->AddPair (title, url);
		}

		if (dia->exec () == QDialog::Accepted)
		{
			RestoredURLs_ = dia->GetSelectedURLs ();
			QTimer::singleShot (2000, this, SLOT (restorePages ()));
		}
	}
	else
	{
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			RestoredURLs_ << settings.value ("URL").toString ();
		}
		QTimer::singleShot (2000, this, SLOT (restorePages ()));
	}
	settings.endArray ();
}

void Core::ScheduleSaveSession ()
{
	if (SaveSessionScheduled_)
		return;

	QTimer::singleShot (1000, this, SLOT (saveSession ()));

	SaveSessionScheduled_ = true;
}

void Core::HandleHistory (QWebView *view)
{
	QString url = view->url ().isValid () ?
		view->url ().toString () : tr ("Unknown");

	HistoryModel_->AddItem (view->title (),
			url, QDateTime::currentDateTime ());
}

void Core::gotUnsupportedContent ()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
	QByteArray data = reply->readAll ();

	LeechCraft::TaskParameters parameters = LeechCraft::Autostart;

	for (QObjectList::const_iterator i = Downloaders_.begin (),
			end = Downloaders_.end (); i != end; ++i)
	{
		IDownload *downloader = qobject_cast<IDownload*> (*i);
		if (downloader->CouldDownload (data, parameters))
		{
			QString directory = QFileDialog::getExistingDirectory (0,
					tr ("Save external data"),
					XmlSettingsManager::Instance ()->
						Property ("ExternalDataSaveLocation",
							QDesktopServices::storageLocation (
								QDesktopServices::DocumentsLocation))
								.toString ());
			if (directory.isEmpty ())
				return;

			LeechCraft::DownloadParams dParams =
			{
				data,
				directory
			};

			XmlSettingsManager::Instance ()->setProperty ("ExternalDataSaveLocation", directory);

			downloader->AddJob (dParams, parameters);
			break;
		}
	}
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

	ScheduleSaveSession ();
}

void Core::handleURLChanged (const QString& newURL)
{
	emit changeTabName (dynamic_cast<QWidget*> (sender ()),
			tr ("Loading %1").arg (QUrl (newURL).host ()));
	HandleHistory (dynamic_cast<BrowserWidget*> (sender ())->GetView ());

	ScheduleSaveSession ();
}

void Core::handleIconChanged (const QIcon& newIcon)
{
	emit changeTabIcon (dynamic_cast<QWidget*> (sender ()), newIcon);
}

void Core::handleNeedToClose ()
{
	BrowserWidget *w = dynamic_cast<BrowserWidget*> (sender ());
	emit removeTab (w);

	Widgets_.erase (std::find (Widgets_.begin (), Widgets_.end (), w));
	w->deleteLater ();

	ScheduleSaveSession ();
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

	DoCommonAuth (msg, authen);
}

void Core::handleProxyAuthentication (const QNetworkProxy& proxy, QAuthenticator *authen)
{
	QString msg = tr ("The proxy <br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (proxy.hostName () + ":" + QString::number (proxy.port ()))
		.arg (authen->realm ());
	msg = msg.left (200);

	DoCommonAuth (msg, authen);
}

void Core::handleSslErrors (QNetworkReply *reply, const QList<QSslError>& errors)
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	settings.beginGroup ("SSL exceptions");
	QStringList keys = settings.allKeys ();
	if (keys.contains (reply->url ().toString ())) 
	{
		if (settings.value (reply->url ().toString ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else if (keys.contains (reply->url ().host ()))
	{
		if (settings.value (reply->url ().host ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else
	{
		QString msg = tr ("The URL<br /><code>%1</code><br />has SSL errors."
				" What do you want to do?")
			.arg (reply->url ().toString ());
		std::auto_ptr<SslErrorsDialog> dia (
				new SslErrorsDialog (msg,
					errors,
					qApp->activeWindow ())
				);

		bool ignore = (dia->exec () == QDialog::Accepted);
		if (ignore)
			reply->ignoreSslErrors ();

		SslErrorsDialog::RememberChoice choice;

		if (choice != SslErrorsDialog::RCNot)
		{
			if (choice == SslErrorsDialog::RCFile)
				settings.setValue (reply->url ().toString (),
						ignore);
			else
				settings.setValue (reply->url ().host (),
						ignore);
		}
	}
	settings.endGroup ();
}

void Core::favoriteTagsUpdated (const QStringList& tags)
{
	XmlSettingsManager::Instance ()->setProperty ("FavoriteTags", tags);
}

void Core::saveSession ()
{
	int pos = 0;
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	settings.beginWriteArray ("Saved session");
	settings.remove ("");
	for (widgets_t::const_iterator i = Widgets_.begin (),
			end = Widgets_.end (); i != end; ++i)
	{
		settings.setArrayIndex (pos++);
		settings.setValue ("Title", (*i)->GetView ()->title ());
		settings.setValue ("URL", (*i)->GetView ()->url ().toString ());
	}
	settings.endArray ();

	SaveSessionScheduled_ = false;
}

void Core::restorePages ()
{
	for (QStringList::const_iterator i = RestoredURLs_.begin (),
			end = RestoredURLs_.end (); i != end; ++i)
		NewURL (*i);
}

void Core::postConstruct ()
{
	bool cleanShutdown = XmlSettingsManager::Instance ()->
		Property ("CleanShutdown", true).toBool ();
	XmlSettingsManager::Instance ()->setProperty ("CleanShutdown", false);

	if (!cleanShutdown)
		RestoreSession (true);
	else if (XmlSettingsManager::Instance ()->
			property ("RestorePreviousSession").toBool ())
		RestoreSession (false);
}

