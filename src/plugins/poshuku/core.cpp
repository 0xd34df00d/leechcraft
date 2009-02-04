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
#include <QInputDialog>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "browserwidget.h"
#include "customwebview.h"
#include "addtofavoritesdialog.h"
#include "xmlsettingsmanager.h"
#include "restoresessiondialog.h"
#include "sqlstoragebackend.h"

using LeechCraft::Util::Proxy;
using LeechCraft::Util::TagsCompletionModel;

Core::Core ()
: SaveSessionScheduled_ (false)
{
	QDir dir = QDir::home ();
	if (!dir.cd (".leechcraft/poshuku") &&
			!dir.mkpath (".leechcraft/poshuku"))
	{
		qCritical () << Q_FUNC_INFO
			<< "could not create neccessary directories for Poshuku";
		return;
	}

	StorageBackend_.reset (new SQLStorageBackend);
	StorageBackend_->Prepare ();

	HistoryModel_.reset (new HistoryModel (this));
	connect (StorageBackend_.get (),
			SIGNAL (added (const HistoryModel::HistoryItem&)),
			HistoryModel_.get (),
			SLOT (handleItemAdded (const HistoryModel::HistoryItem&)));

	URLCompletionModel_.reset (new URLCompletionModel (this));
	connect (StorageBackend_.get (),
			SIGNAL (added (const HistoryModel::HistoryItem&)),
			URLCompletionModel_.get (),
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
	while (Widgets_.begin () != Widgets_.end ())
		delete *Widgets_.begin ();

	HistoryModel_.reset ();
	FavoritesModel_.reset ();
	FavoriteTagsCompletionModel_.reset ();

	XmlSettingsManager::Instance ()->setProperty ("CleanShutdown", true);
	XmlSettingsManager::Instance ()->Release ();
}

void Core::SetProvider (QObject *object, const QString& feature)
{
	Providers_ [feature] = object;
}

bool Core::IsValidURL (const QString& url) const
{
	return QUrl (url).isValid ();
}

BrowserWidget* Core::NewURL (const QString& url, bool raise)
{
	BrowserWidget *widget = new BrowserWidget;

	emit addNewTab (tr (""), widget);

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
	connect (widget,
			SIGNAL (statusBarChanged (const QString&)),
			this,
			SLOT (handleStatusBarChanged (const QString&)));
	connect (widget,
			SIGNAL (gotEntity (const QByteArray&)),
			this,
			SIGNAL (gotEntity (const QByteArray&)));

	widget->SetURL (QUrl (url));

	Widgets_.push_back (widget);

	if (raise)
		emit raiseTab (widget);

	return widget;
}

CustomWebView* Core::MakeWebView (bool invert)
{
	bool raise = true;
	if (XmlSettingsManager::Instance ()->property ("BackgroundNewTabs").toBool ())
		raise = false;

	if (invert)
		raise = !raise;

	return NewURL ("", raise)->GetView ();
}

FavoritesModel* Core::GetFavoritesModel () const
{
	return FavoritesModel_.get ();
}

HistoryModel* Core::GetHistoryModel () const
{
	return HistoryModel_.get ();
}

URLCompletionModel* Core::GetURLCompletionModel () const
{
	return URLCompletionModel_.get ();
}

TagsCompletionModel* Core::GetFavoritesTagsCompletionModel () const
{
	return FavoriteTagsCompletionModel_.get ();
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return NetworkAccessManager_;
}

void Core::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	NetworkAccessManager_ = manager;
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
	{
		qWarning () << Q_FUNC_INFO << widget << "not found in the collection";
		return;
	}

	Widgets_.erase (pos);

	ScheduleSaveSession ();
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
	settings.remove ("");
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
	QString url = view->url ().toString ();

	if (!view->title ().isEmpty () &&
			!url.isEmpty ())
		HistoryModel_->AddItem (view->title (),
				url, QDateTime::currentDateTime ());
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

void Core::handleStatusBarChanged (const QString& msg)
{
	emit statusBarChanged (static_cast<QWidget*> (sender ()), msg);
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

