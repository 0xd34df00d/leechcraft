/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "onlinebookmarks.h"
#include "settings.h"
#include <typeinfo>
#include <QIcon>
#include <QStandardItemModel>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace OnlineBookmarks
				{
					QIcon OnlineBookmarks::GetIcon () const
					{
						return QIcon (":resources/images/defaultpluginicon.svg");
					}

					void OnlineBookmarks::Release ()
					{

					}

					void OnlineBookmarks::SetProvider (QObject *object, const QString& feature)
					{

					}

					QStringList OnlineBookmarks::Uses () const
					{
						return QStringList ();
					}

					QStringList OnlineBookmarks::Needs () const
					{
						return QStringList ();
					}

					QStringList OnlineBookmarks::Provides () const
					{
						return QStringList ();
					}

					QString OnlineBookmarks::GetInfo () const
					{
						return tr ("Sync local bookmarks with your accaunt in the",
								"online bookmarks service such as Delicious.");
					}

					QString OnlineBookmarks::GetName () const
					{
						return "Poshuku OnlineBookmarks";
					}

					QByteArray OnlineBookmarks::GetUniqueID () const
					{
						return "org.LeechCraft.Poshuku.OnlineBookmarks";
					}

					void OnlineBookmarks::SecondInit ()
					{

					}

					void OnlineBookmarks::Init (ICoreProxy_ptr proxy)
					{
						SettingsDialog_.reset (new Util::XmlSettingsDialog);
						SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
								"poshukuonlinebookmarkssettings.xml");
						
						QStandardItemModel *model = new QStandardItemModel;
						QSettings settings (QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_Poshuku_OnlineBookmarks");
						settings.beginGroup ("Accaunts");
						QStringList services = settings.childKeys ();
						
						Q_FOREACH (const QString& item, services)
						{
							QStringList loginList = settings.value (item).toStringList ();
							
							QList<QStandardItem*> itemList;
							
							Q_FOREACH (const QString& login, loginList)
							{	
								QStandardItem *loginItem = new QStandardItem (login);
								loginItem->setCheckable (true);
								itemList << loginItem;
							}
							QStandardItem *service = new QStandardItem (item);
							model->appendRow (service);
							model->itemFromIndex (model->indexFromItem (service))->appendRows (itemList);
						}
						settings.endGroup ();
						
						SettingsDialog_->SetCustomWidget ("Settings", new Settings (model, this));
						
						connect (&Core::Instance (),
								SIGNAL (gotEntity (const LeechCraft::Entity&)),
								this,
								SIGNAL (gotEntity (const LeechCraft::Entity&)));
						
						connect (&Core::Instance (),
								SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
								this,
								SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
					}
					
					QSet<QByteArray> OnlineBookmarks::GetPluginClasses () const
					{
						QSet<QByteArray> result;
						result << "org.LeechCraft.Poshuku.Plugins/1.0";
						return result;
					}
					
					boost::shared_ptr< Util::XmlSettingsDialog > OnlineBookmarks::GetSettingsDialog () const
					{
						return SettingsDialog_;
					}
				};
			};
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_onlinebookmarks, LeechCraft::Plugins::Poshuku::Plugins::OnlineBookmarks::OnlineBookmarks);