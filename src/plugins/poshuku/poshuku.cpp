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
#include <qwebsettings.h>
#include <QHeaderView>
#include <QToolBar>
#include <QDir>
#include <QUrl>
#include <QTextCodec>
#include <QInputDialog>
#include <QMenu>
#include <QMainWindow>
#include <QtDebug>
#include <plugininterface/util.h>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/backendselector.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "browserwidget.h"
#include "cookieseditdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			using LeechCraft::Util::TagsCompletionModel;
			
			void Poshuku::Init (ICoreProxy_ptr coreProxy)
			{
				BrowserWidget::SetParentMultiTabs (this);
				Core::Instance ().setParent (this);
				Core::Instance ().SetProxy (coreProxy);
			
				try
				{
					QWebSettings::setIconDatabasePath (
							LeechCraft::Util::CreateIfNotExists ("poshuku/favicons")
									.absolutePath ()
							);
				}
				catch (const std::runtime_error& e)
				{
					QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
							"LeechCraft",
							e.what ());
				}

				try
				{
					QWebSettings::setOfflineStoragePath (
							LeechCraft::Util::CreateIfNotExists ("poshuku/offlinestorage")
									.absolutePath ()
							);
				}
				catch (const std::runtime_error& e)
				{
					QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
							"LeechCraft",
							e.what ());
				}

				try
				{
					QWebSettings::setOfflineWebApplicationCachePath (
							LeechCraft::Util::CreateIfNotExists ("poshuku/offlinewebappcache")
								.absolutePath ()
							);
				}
				catch (const std::runtime_error& e)
				{
					QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
							"LeechCraft",
							e.what ());
				}
			
				XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"poshukusettings.xml");
				XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
						new LeechCraft::Util::BackendSelector (XmlSettingsManager::Instance ()));

				InitConnections ();
			
				Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));
			
				bool failed = false;
				if (!Core::Instance ().Init ())
				{
					QMessageBox::critical (Core::Instance ().GetProxy ()->GetMainWindow (),
							"LeechCraft",
							tr ("Poshuku failed to initialize properly. "
								"Check logs and talk with the developers. "
								"Or, at least, check the storage backend "
								"settings and restart LeechCraft."));
					failed = true;
				}

				if (failed)
					return;
			
				RegisterSettings ();
			
				connect (Core::Instance ().GetFavoritesModel (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));

				ImportXbel_ = new QAction (tr ("Import XBEL..."),
						this);
				ImportXbel_->setProperty ("ActionIcon", "poshuku_importxbel");
				ExportXbel_ = new QAction (tr ("Export XBEL..."),
						this);
				ExportXbel_->setProperty ("ActionIcon", "poshuku_exportxbel");
				CheckFavorites_ = new QAction (tr ("Check favorites..."),
						this);
				CheckFavorites_->setProperty ("ActionIcon", "poshuku_checkfavorites");

				connect (ImportXbel_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (importXbel ()));
				connect (ExportXbel_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (exportXbel ()));
				connect (CheckFavorites_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleCheckFavorites ()));

				const IShortcutProxy *proxy = coreProxy->GetShortcutProxy ();
				ImportXbel_->setShortcut (proxy->GetShortcut (this, EAImportXbel_));
				ExportXbel_->setShortcut (proxy->GetShortcut (this, EAExportXbel_));
				CheckFavorites_->setShortcut (proxy->GetShortcut (this, EACheckFavorites_));
			
				ToolMenu_ = new QMenu (tr ("Poshuku"),
						Core::Instance ().GetProxy ()->GetMainWindow ());
				ToolMenu_->setIcon (GetIcon ());
				ToolMenu_->addAction (ImportXbel_);
				ToolMenu_->addAction (ExportXbel_);
			}

			void Poshuku::SecondInit ()
			{
				QTimer::singleShot (1000,
						this,
						SLOT (createTabFirstTime ()));
			}
			
			void Poshuku::Release ()
			{
				Core::Instance ().setParent (0);
				Core::Instance ().Release ();
				XmlSettingsDialog_.reset ();
			}
			
			QString Poshuku::GetName () const
			{
				return tr ("Poshuku Browser");
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
			
			QSet<QByteArray> Poshuku::GetExpectedPluginClasses () const
			{
				return Core::Instance ().GetExpectedPluginClasses ();
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
				if (name <= BrowserWidget::ActionMax)
					Core::Instance ().SetShortcut (name, sequence);
				else
				{
					QAction *act = 0;
					switch (name)
					{
						case EAImportXbel_:
							act = ImportXbel_;
							break;
						case EAExportXbel_:
							act = ExportXbel_;
							break;
						case EACheckFavorites_:
							act = CheckFavorites_;
							break;
					}
					if (act)
						act->setShortcut (sequence);
				}
			}
			
			QMap<int, LeechCraft::ActionInfo> Poshuku::GetActionInfo () const
			{
				BrowserWidget bw;
				QMap<int, LeechCraft::ActionInfo> result = bw.GetActionInfo ();
				result [EAImportXbel_] = ActionInfo (ImportXbel_->text (),
						QKeySequence (), ImportXbel_->icon ());
				result [EAExportXbel_] = ActionInfo (ExportXbel_->text (),
						QKeySequence (), ExportXbel_->icon ());
				result [EACheckFavorites_] = ActionInfo (CheckFavorites_->text (),
						QKeySequence (), CheckFavorites_->icon ());
				return result;
			}

			QList<QMenu*> Poshuku::GetToolMenus () const
			{
				QList<QMenu*> result;
				result << ToolMenu_;
				return result;
			}

			QList<QAction*> Poshuku::GetToolActions () const
			{
				QList<QAction*> result;
				result << CheckFavorites_;
				return result;
			}

			void Poshuku::newTabRequested ()
			{
				Core::Instance ().NewURL ("", true);
			}

			void Poshuku::InitConnections ()
			{
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
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));
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
					<< "DNSPrefetchEnabled"
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
			
			void Poshuku::createTabFirstTime ()
			{
				bool firstTime = XmlSettingsManager::Instance ()->
						Property ("FirstTimeRun", true).toBool ();
				bool startWithHome = XmlSettingsManager::Instance ()->
						property ("StartWithHomeTab").toBool ();
				if (firstTime || startWithHome)
					Core::Instance ().NewURL ("about:home", true);
				XmlSettingsManager::Instance ()->
						setProperty ("FirstTimeRun", false);
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
				QWebSettings::globalSettings ()->setAttribute (QWebSettings::DnsPrefetchEnabled,
						XmlSettingsManager::Instance ()->property ("DNSPrefetchEnabled").toBool ());
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
					QMessageBox::information (Core::Instance ().GetProxy ()->GetMainWindow (),
							"LeechCraft",
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
			
			void Poshuku::handleError (const QString& msg)
			{
				emit gotEntity (Util::MakeNotification ("Poshuku", msg, PWarning_));
			}
			
			void Poshuku::handleNewTab ()
			{
				Core::Instance ().NewURL ("", true);
			}
			
			void Poshuku::handleSettingsClicked (const QString& name)
			{
				if (name == "CookiesEdit")
				{
					CookiesEditDialog *dia =
							new CookiesEditDialog (Core::Instance ()
									.GetProxy ()->GetMainWindow ());
					dia->show ();
				}
				else if (name == "ClearIconDatabase")
					QWebSettings::clearIconDatabase ();
				else if (name == "ClearMemoryCaches")
					QWebSettings::clearMemoryCaches ();
				else
					qWarning () << Q_FUNC_INFO
						<< "unknown name"
						<< name;
			}

			void Poshuku::handleCheckFavorites ()
			{
				Core::Instance ().CheckFavorites ();
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, LeechCraft::Plugins::Poshuku::Poshuku);

