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

#include "hub.h"
#include <QAction>
#include <plugininterface/util.h>
#include "hubsortfiltermodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			Hub::UserInfo::UserInfo (const dcpp::Identity& id,
					const dcpp::UserPtr& u)
			: UserInfoBase (u)
			{
				for (int i = 0; i < Hub::CMax; ++i)
					Columns_ << QString ();
				Update (id);
			}

			const dcpp::Identity& Hub::UserInfo::GetIdentity () const
			{
				return Identity_;
			}

			void Hub::UserInfo::Update (const dcpp::Identity& id)
			{
				Columns_ [Hub::CNick] = Util::FromStdString (id.getNick ());
				Columns_ [Hub::CShared] = Util::MakePrettySize (id.getBytesShared ());
				Columns_ [Hub::CDescription] = Util::FromStdString (id.getDescription ());
				Columns_ [Hub::CTag] = Util::FromStdString (id.getTag ());
				Columns_ [Hub::CConnection] = Util::FromStdString (id.getConnection ());
				Columns_ [Hub::CIP] = Util::FromStdString (id.getIp ());
				Columns_ [Hub::CEmail] = Util::FromStdString (id.getEmail ());
				Columns_ [Hub::CCID] = Util::FromStdString (id.getUser ()->getCID ().toBase32 ());

				Identity_ = id;
			}

			QVariant Hub::UserInfo::Data (int column, int role) const
			{
				switch (role)
				{
					case Qt::DisplayRole:
						return Columns_.at (column);
					default:
						return QVariant ();
				}
			}

			Hub::Hub (const QString& aUrl, QWidget *parent)
			: QWidget (parent)
			, UsersModel_ (new Util::ListModel (QStringList (), this))
			, ProxyModel_ (new HubSortFilterModel (this))
			, URL_ (aUrl)
			{
				QStringList headers;
				headers << tr ("Nick")
					<< tr ("Shared")
					<< tr ("Description")
					<< tr ("Tag")
					<< tr ("Connection")
					<< tr ("IP")
					<< tr ("E-mail")
					<< tr ("CID");

				UsersModel_->SetHeaders (headers);
				ProxyModel_->setSourceModel (UsersModel_);

				Ui_.UsersView_->setModel (ProxyModel_);
				Ui_.UsersView_->addAction (Ui_.ActionGetFileList_);
				Ui_.UsersView_->addAction (Ui_.ActionBrowseFileList_);
				Ui_.UsersView_->addAction (Ui_.ActionMatchQueue_);
				Ui_.UsersView_->addAction (Ui_.ActionSendPM_);
				Ui_.UsersView_->addAction (Ui_.ActionGrantExtraSlot_);
				Ui_.UsersView_->addAction (Ui_.ActionAddToFavorites_);
				Ui_.UsersView_->addAction (Ui_.ActionRemoveFromAll_);

				Ui_.Splitter_->setStretchFactor (0, 3);
				Ui_.Splitter_->setStretchFactor (1, 1);
			}

			Hub::~Hub ()
			{
			}

			void Hub::on_ActionGetList__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						GetList (URL_.toStdString ());
			}

			void Hub::on_ActionBrowseFileList__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						BrowseList (URL_.toStdString ());
			}

			void Hub::on_ActionMatchQueue__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						MatchQueue (URL_.toStdString ());
			}

			void Hub::on_ActionSendPM__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						PM (URL_.toStdString ());
			}

			void Hub::on_ActionGrantExtraSlot__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						Grant (URL_.toStdString ());
			}

			void Hub::on_ActionAddToFavorites__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						AddFav ();
			}

			void Hub::on_ActionRemoveFromAll__triggered ()
			{
				Q_FOREACH (QModelIndex i,
						Ui_.UsersView_->selectionModel ()->selectedRows ())
					UsersModel_->GetItem<UserInfo> (ProxyModel_->mapToSource (i))->
						RemoveFromQueue ();
			}

			void Hub::UpdateUser (const dcpp::Identity& id)
			{
				for (int i = 0, size = UsersModel_->rowCount (); i < size; ++i)
				{
					UserInfo *ui = UsersModel_->GetItem<UserInfo> (i);
					if (ui->GetUser () == id.getUser ())
					{
						ui->Update (id);
						UsersModel_->Update (i);
						return;
					}
				}
				UsersModel_->Insert (new UserInfo (id, id.getUser ()));
			}

			void Hub::RemoveUser (const dcpp::UserPtr& user)
			{
				for (int i = 0, size = UsersModel_->rowCount (); i < size; ++i)
				{
					UserInfo *ui = UsersModel_->GetItem<UserInfo> (i);
					if (ui->GetUser () == user)
					{
						UsersModel_->Remove (i);
						return;
					}
				}
			}

			void Hub::on (dcpp::ClientListener::UserUpdated,
					dcpp::Client*, const dcpp::OnlineUser& u) throw ()
			{
				const dcpp::Identity& id = u.getIdentity ();
				if (!id.isHidden ())
					UpdateUser (id);
			}

			void Hub::on (dcpp::ClientListener::UsersUpdated,
					dcpp::Client*, const dcpp::OnlineUser::List& list) throw ()
			{
				for (dcpp::OnlineUser::List::const_iterator i = list.begin (),
						end = list.end (); i != end; ++i)
				{
					const dcpp::Identity& id = (*i)->getIdentity ();
					if (!id.isHidden ())
						UpdateUser (id);
				}
			}

			void Hub::on (dcpp::ClientListener::UserRemoved,
					dcpp::Client*, const dcpp::OnlineUser& u) throw ()
			{
				RemoveUser (u.getIdentity ().getUser ());
			}

			void Hub::on (dcpp::ClientListener::Message,
					dcpp::Client*,
					const dcpp::OnlineUser& from,
					const std::string& msg,
					bool) throw ()
			{
				emit message (tr ("<%1> %2")
						.arg (Util::FromStdString (from.getIdentity ().getNick ()))
						.arg (Util::FromStdString (msg)));
			}
		};
	};
};

