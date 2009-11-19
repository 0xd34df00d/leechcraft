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

#include "networkmonitor.h"
#include <typeinfo>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <plugininterface/util.h>
#include "requestmodel.h"
#include "headermodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("networkmonitor"));

				NetworkAccessManager_ = proxy->GetNetworkAccessManager ();

				Ui_.setupUi (this);
				connect (Ui_.SearchString_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (filterUpdated ()));
				connect (Ui_.SearchType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (filterUpdated ()));

				ProxyModel_ = new QSortFilterProxyModel (this);
				ProxyModel_->setDynamicSortFilter (true);

				Model_ = new RequestModel (this);
				ProxyModel_->setSourceModel (Model_);
				ProxyModel_->setFilterKeyColumn (3);
				Ui_.RequestsView_->setModel (ProxyModel_);
				connect (Ui_.RequestsView_->selectionModel (),
						SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
						this,
						SLOT (handleCurrentChanged (const QModelIndex&)));

				Ui_.RequestHeadersView_->setModel (Model_->GetRequestHeadersModel ());
				Ui_.ReplyHeadersView_->setModel (Model_->GetReplyHeadersModel ());

				connect (Ui_.ClearFinished_,
						SIGNAL (toggled (bool)),
						Model_,
						SLOT (setClear (bool)));

				connect (NetworkAccessManager_,
						SIGNAL (requestCreated (QNetworkAccessManager::Operation,
								const QNetworkRequest&, QNetworkReply*)),
						Model_,
						SLOT (handleRequest (QNetworkAccessManager::Operation,
								const QNetworkRequest&, QNetworkReply*)));

				QAction *showAction = new QAction (tr ("Network monitor..."),
						this);
				showAction->setIcon (GetIcon ());
				connect (showAction,
						SIGNAL (triggered ()),
						this,
						SLOT (show ()));
				Actions_.push_back (showAction);
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
				qDeleteAll (Actions_);
			}

			QString Plugin::GetName () const
			{
				return "NetworkMonitor";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Monitors HTTP network requests and responses.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/networkmonitor.svg");
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			QList<QAction*> Plugin::GetActions () const
			{
				return Actions_;
			}

			void Plugin::handleCurrentChanged (const QModelIndex& index)
			{
				Model_->handleCurrentChanged (ProxyModel_->mapToSource (index));
			}
			
			void Plugin::filterUpdated ()
			{
				QString search = Ui_.SearchString_->text ();
				switch (Ui_.SearchType_->currentIndex ())
				{
					case 0:
						ProxyModel_->setFilterFixedString (search);
						break;
					case 1:
						ProxyModel_->setFilterWildcard (search);
						break;
					case 2:
						ProxyModel_->setFilterRegExp (search);
						break;
					default:
						qWarning () << Q_FUNC_INFO
							<< "unknown search type"
							<< Ui_.SearchType_->currentIndex ()
							<< Ui_.SearchType_->currentText ();
						break;
				}
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_networkmonitor, LeechCraft::Plugins::NetworkMonitor::Plugin);

