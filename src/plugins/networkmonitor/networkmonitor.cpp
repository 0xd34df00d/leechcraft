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
#include <plugininterface/util.h>
#include "requestmodel.h"
#include "headermodel.h"

using namespace LeechCraft::Plugins;
using namespace LeechCraft::Plugins::NetworkMonitor;

void LeechCraft::Plugins::NetworkMonitor::Plugin::Init (ICoreProxy_ptr proxy)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("networkmonitor"));

	NetworkAccessManager_ = proxy->GetNetworkAccessManager ();

	Ui_.setupUi (this);

	Model_ = new RequestModel (this);
	Ui_.RequestsView_->setModel (Model_);
	connect (Ui_.RequestsView_->selectionModel (),
			SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
			Model_,
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
			Ui_.RequestsView_->model (),
			SLOT (handleRequest (QNetworkAccessManager::Operation,
					const QNetworkRequest&, QNetworkReply*)));

	QAction *showAction = new QAction (tr ("Network monitor..."),
			this);
	showAction->setProperty ("ActionIcon", "networkmonitor_plugin");
	connect (showAction,
			SIGNAL (triggered ()),
			this,
			SLOT (show ()));
	Actions_.push_back (showAction);
}

void Plugin::Release ()
{
	qDeleteAll (Actions_);
}

QString Plugin::GetName () const
{
	return "NetworkMonitor";
}

QString LeechCraft::Plugins::NetworkMonitor::Plugin::GetInfo () const
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

Q_EXPORT_PLUGIN2 (leechcraft_networkmonitor, Plugin);

