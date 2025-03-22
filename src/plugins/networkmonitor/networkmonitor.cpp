/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkmonitor.h"
#include <typeinfo>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QNetworkAccessManager>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "requestmodel.h"
#include "headermodel.h"

namespace LC
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
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

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.NetworkMonitor";
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
				return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
			}

			QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
			{
				if (place == ActionsEmbedPlace::ToolsMenu)
					return Actions_;

				return QList<QAction*> ();
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
						ProxyModel_->setFilterRegularExpression (search);
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

LC_EXPORT_PLUGIN (leechcraft_networkmonitor, LC::Plugins::NetworkMonitor::Plugin);
