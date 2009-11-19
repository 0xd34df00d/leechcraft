/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "poshuku.h"
#include <stdexcept>
#include <QMessageBox>
#include <QWebSettings>
#include <QHeaderView>
#include <QToolBar>
#include <QDir>
#include <QUrl>
#include <QTextCodec>
#include <QInputDialog>
#include <QtDebug>
#include <plugininterface/util.h>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/backendselector.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "favoritesdelegate.h"
#include "historymodel.h"
#include "favoritesmodel.h"
#include "browserwidget.h"
#include "cookieseditdialog.h"
#include "editbookmarkdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			using LeechCraft::Util::TagsCompleter;
			using LeechCraft::Util::TagsCompletionModel;
			
			void Poshuku::Init (ICoreProxy_ptr coreProxy)
			{
				Core::Instance ().setParent (this);
				Core::Instance ().SetProxy (coreProxy);
			
				Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));
				Ui_.setupUi (this);
			
				Core::Instance ().ConnectSignals (Ui_.MainView_);
				Ui_.MainView_->InitShortcuts ();
			
				RegisterSettings ();
			
				try
				{
					QWebSettings::setIconDatabasePath (
							LeechCraft::Util::CreateIfNotExists ("poshuku/favicons").absolutePath ()
							);
				}
				catch (const std::runtime_error& e)
				{
					QMessageBox::warning (0,
							tr ("LeechCraft"),
							e.what ());
				}
				try
				{
					QWebSettings::setOfflineStoragePath (
							LeechCraft::Util::CreateIfNotExists ("poshuku/offlinestorage").absolutePath ()
							);
				}
				catch (const std::runtime_error& e)
				{
					QMessageBox::warning (0,
							tr ("LeechCraft"),
							e.what ());
				}
			
				XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"poshukusettings.xml");
				XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
						new LeechCraft::Util::BackendSelector (XmlSettingsManager::Instance ()));
			
				connect (XmlSettingsDialog_.get (),
						SIGNAL (pushButtonClicked (const QString&)),
						this,
						SLOT (handleSettingsClicked (const QString&)));
			
				connect (&Core::Instance (),
						SIGNAL (addNewTab (const QString&, QWidget*)),
						this,
						SIGNAL (addNewTab (const QString&, QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (changeTabName (QWidget*, const QString&)),
						this,
						SIGNAL (changeTabName (QWidget*, const QString&)));
				connect (&Core::Instance (),
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
						this,
						SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
				connect (&Core::Instance (),
						SIGNAL (changeTooltip (QWidget*, QWidget*)),
						this,
						SIGNAL (changeTooltip (QWidget*, QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (statusBarChanged (QWidget*, const QString&)),
						this,
						SIGNAL (statusBarChanged (QWidget*, const QString&)));
				connect (&Core::Instance (),
						SIGNAL (raiseTab (QWidget*)),
						this,
						SIGNAL (raiseTab (QWidget*)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
				connect (&Core::Instance (),
						SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)),
						this,
						SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)));
				connect (&Core::Instance (),
						SIGNAL (downloadFinished (const QString&)),
						this,
						SIGNAL (downloadFinished (const QString&)));
				connect (&Core::Instance (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));
			
				if (!Core::Instance ().Init ())
				{
					QMessageBox::critical (this,
							tr ("LeechCraft"),
							tr ("Poshuku failed to initialize properly. "
								"Check logs and talk with the developers. "
								"Or, at least, check the storage backend "
								"settings and restart LeechCraft."));
					setEnabled (false);
					Ui_.MainView_->GetToolBar ()->setEnabled (false);
					return;
				}
			
				SetupFavoritesFilter ();
				SetupHistoryFilter ();
			
				connect (Core::Instance ().GetFavoritesModel (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));
			
				QHeaderView *itemsHeader = Ui_.FavoritesView_->header ();
				QFontMetrics fm = fontMetrics ();
				itemsHeader->resizeSection (0,
						fm.width ("Average site title can be very big, it's also the "
							"most important part, so it's priority is the biggest."));
				itemsHeader->resizeSection (1,
						fm.width ("Average URL could be very very long, but we don't account this."));
				itemsHeader->resizeSection (2,
						fm.width ("Average tags list size should be like this."));
			
				itemsHeader = Ui_.HistoryView_->header ();
				itemsHeader->resizeSection (0,
						fm.width ("Average site title can be very big, it's also the "
							"most important part, so it's priority is the biggest."));
				itemsHeader->resizeSection (1,
						fm.width (QDateTime::currentDateTime ().toString () + " space"));
				itemsHeader->resizeSection (2,
						fm.width ("Average URL could be very very long, but we don't account this."));
			}

			void Poshuku::SecondInit ()
			{
			}
			
			void Poshuku::Release ()
			{
				Core::Instance ().setParent (0);
				Core::Instance ().Release ();
				XmlSettingsDialog_.reset ();
				FavoritesFilterLineCompleter_.reset ();
				FavoritesFilterModel_.reset ();
			}
			
			QString Poshuku::GetName () const
			{
				return "Poshuku";
			}
			
			QString Poshuku::GetInfo () const
			{
				return tr ("Simple yet functional web browser");
			}
			
			QStringList Poshuku::Provides () const
			{
				return QStringList ("webbrowser");
			}
			
			QStringList Poshuku::Needs () const
			{
				return QStringList ("*");
			}
			
			QStringList Poshuku::Uses () const
			{
				return QStringList ();
			}
			
			void Poshuku::SetProvider (QObject *object, const QString& feature)
			{
				Core::Instance ().SetProvider (object, feature);
			}
			
			QIcon Poshuku::GetIcon () const
			{
				return QIcon (":/resources/images/poshuku.svg");
			}
			
			QWidget* Poshuku::GetTabContents ()
			{
				return this;
			}
			
			QToolBar* Poshuku::GetToolBar () const
			{
				return Ui_.MainView_->GetToolBar ();
			}

			void Poshuku::Remove ()
			{
			}

			void Poshuku::NewTabRequested ()
			{
				Ui_.MainView_->NewTabRequested ();
			}
			
			QByteArray Poshuku::GetExpectedPluginClass () const
			{
				return Core::Instance ().GetExpectedPluginClass ();
			}
			
			void Poshuku::AddPlugin (QObject *plugin)
			{
				Core::Instance ().AddPlugin (plugin);
			}
			
			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Poshuku::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			bool Poshuku::CouldHandle (const LeechCraft::DownloadEntity& e) const
			{
				if (!(e.Parameters_ & FromUserInitiated) ||
						e.Parameters_ & Internal)
					return false;

				if (!e.Entity_.canConvert<QUrl> ())
					return false;

				QUrl url = e.Entity_.toUrl ();
				return (url.isValid () &&
					(url.scheme () == "http" || url.scheme () == "https"));
			}

			void Poshuku::Handle (LeechCraft::DownloadEntity e)
			{
				QUrl url = e.Entity_.toUrl ();
				Core::Instance ().NewURL (url, true);
			}
			
			void Poshuku::Open (const QString& link)
			{
				Core::Instance ().NewURL (link);
			}
			
			IWebWidget* Poshuku::GetWidget () const
			{
				return Core::Instance ().GetWidget ();
			}

			QWebView* Poshuku::CreateWindow ()
			{
				return Core::Instance ().MakeWebView ();
			}
			
			void Poshuku::SetShortcut (int name, const QKeySequence& sequence)
			{
				Ui_.MainView_->SetShortcut (name, sequence);
				Core::Instance ().SetShortcut (name, sequence);
			}
			
			QMap<int, LeechCraft::ActionInfo> Poshuku::GetActionInfo () const
			{
				return Ui_.MainView_->GetActionInfo ();
			}
			
			void Poshuku::RegisterSettings ()
			{
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
					<< "AllowJavascript"
					<< "AllowJava"
					<< "AllowPlugins"
					<< "JavascriptCanOpenWindows"
					<< "JavascriptCanAccessClipboard"
					<< "UserStyleSheet"
					<< "OfflineStorageDB"
					<< "LocalStorageDB"
					<< "OfflineWebApplicationCache";
				XmlSettingsManager::Instance ()->RegisterObject (viewerSettings,
						this, "viewerSettingsChanged");

				XmlSettingsManager::Instance ()->RegisterObject ("DeveloperExtrasEnabled",
						this, "developerExtrasChanged");
			
				viewerSettingsChanged ();
				developerExtrasChanged ();
			
				QList<QByteArray> cacheSettings;
				cacheSettings << "MaximumPagesInCache"
					<< "MinDeadCapacity"
					<< "MaxDeadCapacity"
					<< "TotalCapacity"
					<< "OfflineStorageQuota";
				XmlSettingsManager::Instance ()->RegisterObject (cacheSettings,
						this, "cacheSettingsChanged");
			
				cacheSettingsChanged ();
			}
			
			void Poshuku::SetupFavoritesFilter ()
			{
				FavoritesFilterModel_.reset (new FilterModel (this));
				FavoritesFilterModel_->setSourceModel (Core::Instance ().GetFavoritesModel ());
				FavoritesFilterModel_->setDynamicSortFilter (true);
				Ui_.FavoritesView_->setModel (FavoritesFilterModel_.get ());
				Ui_.FavoritesView_->setItemDelegate (new FavoritesDelegate (this));
				Ui_.FavoritesView_->addAction (Ui_.ActionEditBookmark_);
				Ui_.FavoritesView_->addAction (Ui_.ActionChangeURL_);
				Ui_.FavoritesView_->addAction (Ui_.ActionDeleteBookmark_);
				connect (Ui_.FavoritesView_,
						SIGNAL (deleteSelected (const QModelIndex&)),
						this,
						SLOT (translateRemoveFavoritesItem (const QModelIndex&)));
			
				FavoritesFilterLineCompleter_.reset (
						new TagsCompleter (Ui_.FavoritesFilterLine_, this)
						);
				Ui_.FavoritesFilterLine_->AddSelector ();
				connect (Ui_.FavoritesFilterLine_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (updateFavoritesFilter ()));
				connect (Ui_.FavoritesFilterType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (updateFavoritesFilter ()));
				connect (Ui_.FavoritesFilterCaseSensitivity_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (updateFavoritesFilter ()));
			}
			
			void Poshuku::SetupHistoryFilter ()
			{
				HistoryFilterModel_.reset (new HistoryFilterModel (this));
				HistoryFilterModel_->setSourceModel (Core::Instance ().GetHistoryModel ());
				Core::Instance ().GetHistoryModel ()->setParent (HistoryFilterModel_.get ());
				HistoryFilterModel_->setDynamicSortFilter (true);
				Ui_.HistoryView_->setModel (HistoryFilterModel_.get ());
			
				connect (Ui_.HistoryFilterLine_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (updateHistoryFilter ()));
				connect (Ui_.HistoryFilterType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (updateHistoryFilter ()));
				connect (Ui_.HistoryFilterCaseSensitivity_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (updateHistoryFilter ()));
			}
			
			void Poshuku::on_HistoryView__activated (const QModelIndex& index)
			{
				if (!index.parent ().isValid ())
					return;

				Core::Instance ().NewURL (index.sibling (index.row (),
							HistoryModel::ColumnURL).data ().toString ());
			}
			
			void Poshuku::on_FavoritesView__activated (const QModelIndex& index)
			{
				Ui_.ActionEditBookmark_->setEnabled (index.isValid ());
				Ui_.ActionChangeURL_->setEnabled (index.isValid ());
				Ui_.ActionDeleteBookmark_->setEnabled (index.isValid ());
				Core::Instance ().NewURL (index.sibling (index.row (),
							FavoritesModel::ColumnURL).data ().toString ());
			}
			
			void Poshuku::on_OpenInTabs__released ()
			{
				for (int i = 0, size = FavoritesFilterModel_->rowCount (); 
						i < size; ++i)
					Core::Instance ().NewURL (FavoritesFilterModel_->
							index (i, FavoritesModel::ColumnURL).data ().toString ());
			}
			
			void Poshuku::on_ActionEditBookmark__triggered ()
			{
				QModelIndex current = Ui_.FavoritesView_->
					selectionModel ()->currentIndex ();
				if (!current.isValid ())
					return;
				QModelIndex source = FavoritesFilterModel_->mapToSource (current);

				EditBookmarkDialog dia (source, this);
				if (dia.exec () != QDialog::Accepted)
					return;

				FavoritesModel *model = Core::Instance ().GetFavoritesModel ();
				model->setData (source.sibling (source.row (),
							FavoritesModel::ColumnTitle),
						dia.GetTitle ());
				model->setData (source.sibling (source.row (),
							FavoritesModel::ColumnTags),
						dia.GetTags ());
			}

			void Poshuku::on_ActionChangeURL__triggered ()
			{
				QModelIndex current = Ui_.FavoritesView_->
					selectionModel ()->currentIndex ();
				if (!current.isValid ())
					return;
				QModelIndex source = FavoritesFilterModel_->mapToSource (current);

				QString title = source.sibling (source.row (),
						FavoritesModel::ColumnTitle).data ().toString ();

				QString currentURL = source.sibling (source.row (),
						FavoritesModel::ColumnURL).data ().toString ();

				bool ok = false;

				QString newURL = QInputDialog::getText (this,
						tr ("Change URL"),
						tr ("Enter new URL for<br />%1")
							.arg (title),
						QLineEdit::Normal,
						currentURL,
						&ok);

				if (!ok)
					return;

				if (newURL.isEmpty ())
					QMessageBox::critical (this,
							tr ("LeechCraft"),
							tr ("URL of a bookmark can't be empty."));

				Core::Instance ().GetFavoritesModel ()->ChangeURL (source, newURL);
			}

			void Poshuku::on_ActionDeleteBookmark__triggered ()
			{
				QModelIndex current = Ui_.FavoritesView_->
					selectionModel ()->currentIndex ();
				if (!current.isValid ())
					return;

				translateRemoveFavoritesItem (current);
			}
			
			void Poshuku::translateRemoveFavoritesItem (const QModelIndex& sourceIndex)
			{
				QModelIndex index = FavoritesFilterModel_->mapToSource (sourceIndex);
				Core::Instance ().GetFavoritesModel ()->removeItem (index);
			}
			
			void Poshuku::viewerSettingsChanged ()
			{
				QWebSettings::globalSettings ()->setFontFamily (QWebSettings::StandardFont,
						XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
				QWebSettings::globalSettings ()->setFontFamily (QWebSettings::FixedFont,
						XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
				QWebSettings::globalSettings ()->setFontFamily (QWebSettings::SerifFont,
						XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
				QWebSettings::globalSettings ()->setFontFamily (QWebSettings::SansSerifFont,
						XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
				QWebSettings::globalSettings ()->setFontFamily (QWebSettings::CursiveFont,
						XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
				QWebSettings::globalSettings ()->setFontFamily (QWebSettings::FantasyFont,
						XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());
			
				QWebSettings::globalSettings ()->setFontSize (QWebSettings::MinimumFontSize,
						XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
				QWebSettings::globalSettings ()->setFontSize (QWebSettings::DefaultFontSize,
						XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
				QWebSettings::globalSettings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
						XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::AutoLoadImages,
						XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptEnabled,
						XmlSettingsManager::Instance ()->property ("AllowJavascript").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavaEnabled,
						XmlSettingsManager::Instance ()->property ("AllowJava").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::PluginsEnabled,
						XmlSettingsManager::Instance ()->property ("AllowPlugins").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptCanOpenWindows,
						XmlSettingsManager::Instance ()->property ("JavascriptCanOpenWindows").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptCanAccessClipboard,
						XmlSettingsManager::Instance ()->property ("JavascriptCanAccessClipboard").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::OfflineStorageDatabaseEnabled,
						XmlSettingsManager::Instance ()->property ("OfflineStorageDB").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::OfflineWebApplicationCacheEnabled,
						XmlSettingsManager::Instance ()->property ("OfflineWebApplicationCache").toBool ());
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::LocalStorageDatabaseEnabled,
						XmlSettingsManager::Instance ()->property ("LocalStorageDB").toBool ());
				QWebSettings::globalSettings ()->setUserStyleSheetUrl (QUrl (XmlSettingsManager::
							Instance ()->property ("UserStyleSheet").toString ()));
			}

			void Poshuku::developerExtrasChanged ()
			{
				bool enabled = XmlSettingsManager::Instance ()->
					property ("DeveloperExtrasEnabled").toBool ();
				QWebSettings::globalSettings ()->
					setAttribute (QWebSettings::DeveloperExtrasEnabled, enabled);
				if (enabled && sender ())
					QMessageBox::information (this,
							tr ("LeechCraft"),
							tr ("Please note that Developer Extras would work correctly "
								"only for pages that are loaded after enabling."));
			}
			
			void Poshuku::cacheSettingsChanged ()
			{
				QWebSettings::setMaximumPagesInCache (XmlSettingsManager::Instance ()->
						property ("MaximumPagesInCache").toInt ());
				QWebSettings::setObjectCacheCapacities (
						XmlSettingsManager::Instance ()->property ("MinDeadCapacity").toDouble () * 1024 * 1024,
						XmlSettingsManager::Instance ()->property ("MaxDeadCapacity").toDouble () * 1024 * 1024,
						XmlSettingsManager::Instance ()->property ("TotalCapacity").toDouble () * 1024 * 1024
						);
				QWebSettings::setOfflineStorageDefaultQuota (XmlSettingsManager::Instance ()->
						property ("OfflineStorageQuota").toInt () * 1024);
			}
			
			void Poshuku::updateFavoritesFilter ()
			{
				int section = Ui_.FavoritesFilterType_->currentIndex ();
				QString text = Ui_.FavoritesFilterLine_->text ();
			
				switch (section)
				{
					case 1:
						FavoritesFilterModel_->setTagsMode (false);
						FavoritesFilterModel_->setFilterWildcard (text);
						break;
					case 2:
						FavoritesFilterModel_->setTagsMode (false);
						FavoritesFilterModel_->setFilterRegExp (text);
						break;
					case 3:
						FavoritesFilterModel_->setTagsMode (true);
						FavoritesFilterModel_->setFilterFixedString (text);
						break;
					default:
						FavoritesFilterModel_->setTagsMode (false);
						FavoritesFilterModel_->setFilterFixedString (text);
						break;
				}
			
				FavoritesFilterModel_->
					setFilterCaseSensitivity ((Ui_.FavoritesFilterCaseSensitivity_->
								checkState () == Qt::Checked) ? Qt::CaseSensitive :
							Qt::CaseInsensitive);
			}
			
			void Poshuku::updateHistoryFilter ()
			{
				int section = Ui_.HistoryFilterType_->currentIndex ();
				QString text = Ui_.HistoryFilterLine_->text ();
			
				switch (section)
				{
					case 1:
						HistoryFilterModel_->setFilterWildcard (text);
						break;
					case 2:
						HistoryFilterModel_->setFilterRegExp (text);
						break;
					default:
						HistoryFilterModel_->setFilterFixedString (text);
						break;
				}
			
				HistoryFilterModel_->
					setFilterCaseSensitivity ((Ui_.HistoryFilterCaseSensitivity_->
								checkState () == Qt::Checked) ? Qt::CaseSensitive :
							Qt::CaseInsensitive);
			}
			
			void Poshuku::handleError (const QString& msg)
			{
				QMessageBox::warning (this,
						tr ("LeechCraft"),
						msg);
			}
			
			void Poshuku::handleNewTab ()
			{
				Core::Instance ().NewURL ("", true);
			}
			
			void Poshuku::handleSettingsClicked (const QString& name)
			{
				if (name == "CookiesEdit")
				{
					CookiesEditDialog *dia = new CookiesEditDialog ();
					dia->show ();
				}
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, LeechCraft::Plugins::Poshuku::Poshuku);

