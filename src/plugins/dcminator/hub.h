/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_DCMINATOR_HUB_H
#define PLUGINS_DCMINATOR_HUB_H
#include <QWidget>
#include <plugininterface/listmodel.h>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Client.h"
#include "dcpp/ClientManager.h"
#include "dcpp/FastAlloc.h"
#include "userinfobase.h"
#include "ui_hub.h"

class QAction;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			class HubSortFilterModel;

			class Hub : public QWidget
					  , private dcpp::ClientListener
			{
				Q_OBJECT

				Ui::Hub Ui_;
				Util::ListModel *UsersModel_;
				HubSortFilterModel *ProxyModel_;
				QString URL_;
				dcpp::Client *Client_;
			public:
				class UserInfo : public UserInfoBase
							   , public Util::ListModelItem
							   , public dcpp::FastAlloc<UserInfo>
				{
					dcpp::Identity Identity_;
					QStringList Columns_;
				public:
					UserInfo (const dcpp::Identity&, const dcpp::UserPtr&);
					void Update (const dcpp::Identity&);
					QVariant Data (int, int) const;

					const dcpp::Identity& GetIdentity () const;
				};

				enum Columns
				{
					CNick,
					CShared,
					CDescription,
					CTag,
					CConnection,
					CIP,
					CEmail,
					CCID,
					CMax
				};

				Hub (const QString& = "", QWidget* = 0);
				virtual ~Hub ();

				void ConnectHub (const QString&);
			private slots:
				void on_ActionGetList__triggered ();
				void on_ActionBrowseFileList__triggered ();
				void on_ActionMatchQueue__triggered ();
				void on_ActionSendPM__triggered ();
				void on_ActionGrantExtraSlot__triggered ();
				void on_ActionAddToFavorites__triggered ();
				void on_ActionRemoveFromAll__triggered ();
				void handleDisconnected ();
				void handlePassword ();
				void sendMessage ();
				void filter (const QString&);
			private:
				void UpdateUser (const dcpp::Identity&);
				void RemoveUser (const dcpp::UserPtr&);

				void on (dcpp::ClientListener::UserUpdated,
						dcpp::Client*, const dcpp::OnlineUser&) throw ();
				void on (dcpp::ClientListener::UsersUpdated,
						dcpp::Client*, const dcpp::OnlineUser::List&) throw ();
				void on (dcpp::ClientListener::UserRemoved,
						dcpp::Client*, const dcpp::OnlineUser&) throw ();
				void on (dcpp::ClientListener::Message,
						dcpp::Client*,
						const dcpp::OnlineUser&,
						const std::string&,
						bool) throw ();
				void on (dcpp::ClientListener::StatusMessage,
						dcpp::Client*,
						const std::string&, bool) throw ();
			signals:
				void message (const QString&);
				void status (const QString&);
				void disconnected ();
				void password ();
			};
		};
	};
};

#endif

