#include "core.h"
#include <algorithm>
#include <memory>
#include <typeinfo>
#include <QString>
#include <QUrl>
#include <QWidget>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
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
#include "xbelparser.h"
#include "xbelgenerator.h"
#include "interfaces/pluginbase.h"

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
			SIGNAL (added (const HistoryItem&)),
			HistoryModel_.get (),
			SLOT (handleItemAdded (const HistoryItem&)));

	URLCompletionModel_.reset (new URLCompletionModel (this));
	connect (StorageBackend_.get (),
			SIGNAL (added (const HistoryItem&)),
			URLCompletionModel_.get (),
			SLOT (handleItemAdded (const HistoryItem&)));

	PluginManager_.reset (new PluginManager (this));

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

QByteArray Core::GetExpectedPluginClass () const
{
	return QByteArray (typeid (LeechCraft::Poshuku::PluginBase).name ());
}

void Core::AddPlugin (QObject *plugin)
{
	PluginManager_->AddPlugin (plugin);
}

QUrl Core::MakeURL (QString url) const
{
	QUrl result = QUrl (url);
	if (result.scheme ().isEmpty ())
	{
		if (!url.count (' ') && url.count ('.'))
			result = QUrl (QString ("http://") + url);
		else
		{
			url.replace (' ', '+');
			result = QUrl (QString ("http://www.google.com/search?q=%1").arg (url));
		}
	}
	return result;
}

BrowserWidget* Core::NewURL (const QString& url, bool raise)
{
	BrowserWidget *widget = new BrowserWidget ();
	widget->SetUnclosers (Unclosers_);
	Widgets_.push_back (widget);

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
			SIGNAL (tooltipChanged (QWidget*)),
			this,
			SLOT (handleTooltipChanged (QWidget*)));
	connect (widget,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
	connect (widget,
			SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)),
			this,
			SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)));

	widget->SetURL (QUrl (url));

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

PluginManager* Core::GetPluginManager () const
{
	return PluginManager_.get ();
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

	QString title = widget->GetView ()->title ();
	if (title.isEmpty ())
		title = widget->GetView ()->url ().toString ();

	if (!title.isEmpty ())
	{
		if (title.size () > 53)
			title = title.left (50) + "...";
		QAction *action = new QAction (widget->GetView ()->icon (),
				title, this);
		action->setData (widget->GetView ()->url ());

		connect (action,
				SIGNAL (triggered ()),
				this,
				SLOT (handleUnclose ()));

		emit newUnclose (action);

		Unclosers_.push_front (action);
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
		else
			saveSession ();
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
			!url.isEmpty () && url != "about:blank")
		HistoryModel_->AddItem (view->title (),
				url, QDateTime::currentDateTime ());
}

void Core::importXbel ()
{
	QString suggestion = XmlSettingsManager::Instance ()->
			Property ("LastXBELOpen", QDir::homePath ()).toString ();
	QString filename = QFileDialog::getOpenFileName (0,
			tr ("Select XBEL file"),
			suggestion,
			tr ("XBEL files (*.xbel);;"
				"All files (*.*)"));

	if (filename.isEmpty ())
		return;

	XmlSettingsManager::Instance ()->setProperty ("LastXBELOpen",
			QFileInfo (filename).absolutePath ());

	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		QMessageBox::critical (0,
				tr ("Error"),
				tr ("Could not open file %1 for reading.")
					.arg (filename));
		return;
	}

	QByteArray data = file.readAll ();

	try
	{
		XbelParser p (data);
	}
	catch (const std::exception& e)
	{
		QMessageBox::critical (0,
				tr ("Error"),
				e.what ());
	}
}

void Core::exportXbel ()
{
	QString suggestion = XmlSettingsManager::Instance ()->
			Property ("LastXBELSave", QDir::homePath ()).toString ();
	QString filename = QFileDialog::getSaveFileName (0,
			tr ("Save XBEL file"),
			suggestion,
			tr ("XBEL files (*.xbel);;"
				"All files (*.*)"));

	if (filename.isEmpty ())
		return;

	if (!filename.endsWith (".xbel"))
		filename.append (".xbel");

	XmlSettingsManager::Instance ()->setProperty ("LastXBELSave",
			QFileInfo (filename).absolutePath ());

	QFile file (filename);
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		QMessageBox::critical (0,
				tr ("Error"),
				tr ("Could not open file %1 for writing.")
					.arg (filename));
		return;
	}

	QByteArray data;
	XbelGenerator g (data);
	file.write (data);
}

void Core::handleUnclose ()
{
	QAction *action = qobject_cast<QAction*> (sender ());
	NewURL (action->data ().toUrl ().toString ());
	Unclosers_.removeAll (action);
	action->deleteLater ();
}

void Core::handleTitleChanged (const QString& newTitle)
{
	emit changeTabName (dynamic_cast<QWidget*> (sender ()), newTitle);

	ScheduleSaveSession ();
}

void Core::handleURLChanged (const QString& newURL)
{
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

	w->deleteLater ();

	ScheduleSaveSession ();
}

void Core::handleAddToFavorites (const QString& title, const QString& url)
{
	std::auto_ptr<AddToFavoritesDialog> dia (new AddToFavoritesDialog (title,
				url,
				GetFavoritesTagsCompletionModel (),
				qApp->activeWindow ()));

	bool result = false;
	do
	{
		if (dia->exec () == QDialog::Rejected)
			return;

		result = FavoritesModel_->AddItem (dia->GetTitle (),
				url, dia->GetTags ());
	}
	while (!result);

	FavoriteTagsCompletionModel_->UpdateTags (dia->GetTags ());
}

void Core::handleStatusBarChanged (const QString& msg)
{
	emit statusBarChanged (static_cast<QWidget*> (sender ()), msg);
}

void Core::handleTooltipChanged (QWidget *tip)
{
	emit changeTooltip (static_cast<QWidget*> (sender ()), tip);
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

	saveSession ();
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

