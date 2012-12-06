/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <memory>
#include "ljaccount.h"
#include <QtDebug>
#include <boost/graph/graph_concepts.hpp>
#include <util/passutils.h>
#include <util/util.h>
#include "core.h"
#include "ljaccountconfigurationwidget.h"
#include "ljaccountconfigurationdialog.h"
#include "ljbloggingplatform.h"
#include "ljprofile.h"
#include "ljxmlrpc.h"
#include "profilewidget.h"
#include "utils.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJAccount::LJAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (qobject_cast<LJBloggingPlatform*> (parent))
	, LJXmlRpc_ (new LJXmlRPC (this, this))
	, Name_ (name)
	, IsValidated_ (false)
	, LJProfile_ (std::make_shared<LJProfile> (this))
	{
		qRegisterMetaType<LJProfileData> ("LJProfileData");
		qRegisterMetaTypeStreamOperators<QList<LJFriendGroup>> ("QList<LJFriendGroup>");
		qRegisterMetaTypeStreamOperators<QList<LJMood>> ("QList<LJMood>");

		connect (LJXmlRpc_,
				SIGNAL (validatingFinished (bool)),
				this,
				SLOT (handleValidatingFinished (bool)));
		connect (LJXmlRpc_,
				SIGNAL (error (int, const QString&)),
				this,
				SLOT (handleXmlRpcError (int, const QString&)));
		connect (LJXmlRpc_,
				SIGNAL (profileUpdated (const LJProfileData&)),
				LJProfile_.get (),
				SLOT (handleProfileUpdate (const LJProfileData&)));
		connect (LJXmlRpc_,
				SIGNAL (entryPosted ()),
				this,
				SIGNAL (entryPosted ()));
		connect (LJXmlRpc_,
				SIGNAL(entryRemoved (int)),
				this,
				SIGNAL (entryRemoved (int)));
		connect (LJXmlRpc_,
				SIGNAL (entryUpdated (int)),
				this,
				SIGNAL (entryUpdated (int)));
		connect (LJXmlRpc_,
				SIGNAL (gotEntries2Backup (QList<LJEvent>)),
				this,
				SLOT (handleGotEntries2Backup (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gettingEntries2BackupFinished ()),
				this,
				SLOT (handleGettingEvents2BackupFinished ()));
		connect (LJXmlRpc_,
				SIGNAL (gotEntries (QList<LJEvent>)),
				this,
				SLOT (handleGotEntries (QList<LJEvent>)));
	}

	QObject* LJAccount::GetObject ()
	{
		return this;
	}

	QObject* LJAccount::GetParentBloggingPlatform () const
	{
		return ParentBloggingPlatform_;
	}

	QString LJAccount::GetAccountName () const
	{
		return Name_;
	}

	QString LJAccount::GetOurLogin () const
	{
		return Login_;
	}

	void LJAccount::RenameAccount (const QString&)
	{
	}

	QByteArray LJAccount::GetAccountID () const
	{
		return ParentBloggingPlatform_->GetBloggingPlatformID () + "_" +
				Login_.toUtf8();
	}

	void LJAccount::OpenConfigurationDialog ()
	{
		std::unique_ptr<LJAccountConfigurationDialog> dia (new LJAccountConfigurationDialog (0));

		if (!Login_.isEmpty ())
			dia->ConfWidget ()->SetLogin (Login_);

		dia->ConfWidget ()->SetPassword (GetPassword ());

		if (dia->exec () == QDialog::Rejected)
			return;

		FillSettings (dia->ConfWidget ());
	}

	bool LJAccount::IsValidated () const
	{
		return IsValidated_;
	}

	QString LJAccount::GetPassword () const
	{
		QString key ("org.LeechCraft.Blogique.PassForAccount/" + GetAccountID ());
		return Util::GetPassword (key, QString (), &Core::Instance ());
	}

	QObject* LJAccount::GetProfile ()
	{
		return LJProfile_.get ();
	}

	void LJAccount::GetLastEntries (int count)
	{
		LJXmlRpc_->GetLastEntries (count);
	}

	namespace
	{
		LJEventProperties GetLJEventPropetriesFromMap (const QVariantMap& map)
		{
			LJEventProperties props;
			props.AdultContent_ = static_cast<AdultContent> (map ["adults"].toInt ());
			props.CommentsManagement_ = static_cast<CommentsManagement> (map ["comment"].toInt ());
			props.CurrentLocation_ = map ["place"].toString ();
			props.CurrentMood_ = map ["mood"].toString ();
			props.CurrentMoodId_ = map ["moodId"].toInt ();
			props.CurrentMusic_ = map ["music"].toString ();
			props.ScreeningComments_ = static_cast<CommentsManagement> (map ["hidecomment"].toInt ());
			props.PostAvatar_ = map ["avatar"].toString ();
			props.ShowInFriendsPage_ = map ["showInFriendsPage"].toBool ();

			return props;
		}

		LJEvent Event2LJEvent (const Event& event)
		{
			LJEvent ljEvent;
			ljEvent.ItemID_ = event.EntryId_;
			ljEvent.Event_ = event.Content_;
			ljEvent.DateTime_ = event.Date_;
			ljEvent.Subject_ = event.Subject_;
			ljEvent.Tags_ = event.Tags_;
			ljEvent.AllowMask_ = event.PostOptions_ ["allowMask"].toUInt ();
			ljEvent.Security_ = static_cast<Access> (event.PostOptions_ ["access"].toInt ());
			ljEvent.Props_ = GetLJEventPropetriesFromMap (event.PostOptions_);
			return ljEvent;
		}
	}

	void LJAccount::RemoveEntry (const Event& event)
	{
		LJXmlRpc_->RemoveEvent (Event2LJEvent (event));
	}

	void LJAccount::UpdateEntry (const Event& event)
	{
		LJXmlRpc_->UpdateEvent (Event2LJEvent (event));
	}

	void LJAccount::FillSettings (LJAccountConfigurationWidget *widget)
	{
		Login_ = widget->GetLogin ();
		const QString& pass = widget->GetPassword ();
		if (!pass.isNull ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + GetAccountID (),
					&Core::Instance ());

		emit accountSettingsChanged ();
		Validate ();
	}

	QByteArray LJAccount::Serialize () const
	{
		quint16 ver = 2;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< Name_
					<< Login_
					<< IsValidated_
					<< LJProfile_->GetProfileData ();
		}

		return result;
	}

	LJAccount* LJAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 ver = 0;
		QDataStream in (data);
		in >> ver;

		if (ver > 2 ||
				ver < 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		QString name;
		in >> name;
		LJAccount *result = new LJAccount (name, parent);
		in >> result->Login_
				>> result->IsValidated_;

		if (ver == 2)
		{
			LJProfileData profile;
			in >> profile;
			result->LJProfile_->handleProfileUpdate (profile);
		}

		return result;
	}

	void LJAccount::Validate ()
	{
		LJXmlRpc_->Validate (Login_, GetPassword ());
	}

	void LJAccount::Init ()
	{
		connect (this,
				SIGNAL (accountValidated (bool)),
				ParentBloggingPlatform_,
				SLOT (handleAccountValidated (bool)));

		connect (this,
				SIGNAL (accountSettingsChanged ()),
				ParentBloggingPlatform_,
				SLOT (saveAccounts ()));
	}

	void LJAccount::AddFriends (const QList<LJFriendEntry_ptr>& friends)
	{
		LJProfile_->AddFriends (friends);
	}

	void LJAccount::AddNewFriend (const QString& username,
			const QString& bgcolor, const QString& fgcolor, uint groupId)
	{
		LJXmlRpc_->AddNewFriend (username, bgcolor, fgcolor, groupId);
	}

	void LJAccount::DeleteFriend (const QString& username)
	{
		LJXmlRpc_->DeleteFriend (username);
	}

	void LJAccount::AddGroup (const QString& name, bool isPublic, int id)
	{
		LJXmlRpc_->AddGroup (name, isPublic, id);
	}

	void LJAccount::DeleteGroup (int id)
	{
		LJXmlRpc_->DeleteGroup (id);
	}

	void LJAccount::handleValidatingFinished (bool success)
	{
		IsValidated_ = success;
		qDebug () << Q_FUNC_INFO
				<< "account"
				<< GetAccountID ()
				<< "validating result is"
				<< IsValidated_;

		emit accountValidated (IsValidated_);
		emit accountSettingsChanged ();
	}

	void LJAccount::handleXmlRpcError (int errorCode, const QString& msgInEng)
	{
		Entity e = Util::MakeNotification ("Blogique",
				tr ("%1 (original message: %2)")
						.arg (MetidaUtils::GetLocalizedErrorMessage (errorCode), msgInEng),
				PWarning_);

		qWarning () << Q_FUNC_INFO
				<< "error code:"
				<< errorCode
				<< "error text:"
				<< msgInEng;

		Core::Instance ().SendEntity (e);
	}

	void LJAccount::updateProfile ()
	{
		LJXmlRpc_->UpdateProfileInfo ();
	}

	void LJAccount::submit (const Event& event)
	{
		LJEvent ljEvent;
		LJEventProperties props;
		const QVariantMap& postOptions = event.PostOptions_;
		const QVariantMap& customData = event.CustomData_;

		ljEvent.Subject_ = event.Subject_;
		ljEvent.Event_ = event.Content_;
		ljEvent.UseJournal_ = event.Target_;
		ljEvent.DateTime_ = event.Date_;
		ljEvent.Tags_ = event.Tags_;
		Access access = static_cast<Access> (postOptions.value ("access").toInt ());
		ljEvent.Security_ = access < Access::MAXAccess ?
			access :
			Access::Public;
		if (access == Access::Custom)
			ljEvent.AllowMask_ = postOptions ["allowMask"].toUInt ();

		AdultContent adultContent = static_cast<AdultContent> (postOptions
				.value ("adults").toInt ());
		props.AdultContent_ = adultContent < AdultContent::MAXAdult ?
			adultContent :
			AdultContent::WithoutAdultContent;

		CommentsManagement managment, screening;
		managment = static_cast<CommentsManagement> (postOptions
				.value ("comment").toInt ());
		screening =  static_cast<CommentsManagement> (postOptions
				.value ("hidecomment").toInt ());

		props.CommentsManagement_ = managment < CommentsManagement::MAXManagment ?
			managment :
			CommentsManagement::Default;
		props.ScreeningComments_ = screening > CommentsManagement::MAXManagment &&
				screening < CommentsManagement::MAXScreening ?
			screening :
			CommentsManagement::ShowComments;

		props.CurrentLocation_ = postOptions.value ("place").toString ();
		props.CurrentMusic_ = postOptions.value ("music").toString ();

		int currentMoodId = postOptions.value ("moodId").toInt ();
		if (!currentMoodId)
			props.CurrentMood_ = postOptions.value ("mood").toString ();
		else
			props.CurrentMoodId_ = currentMoodId;

		props.ShowInFriendsPage_ = postOptions.value ("showInFriendsPage").toBool ();

		props.PostAvatar_ = postOptions.value ("avatar").toString ();

		ljEvent.Props_ = props;
		//TODO
		ljEvent.Event_.append ("<em style=\"font-size: 0.8em;\">Posted via <a href=\"http://leechcraft.org/plugins-blogique\">LeechCraft Blogique</a>.</em>");
		LJXmlRpc_->Submit (ljEvent);
	}

	void LJAccount::backup ()
	{
		LJXmlRpc_->BackupEvents ();
	}

	namespace
	{
		QVariantMap GetPostOptionsMapFromLJEvent (const LJEvent& event)
		{
			QVariantMap map;
			map ["access"] = event.Security_;
			map ["allowMask"] = event.AllowMask_;
			map ["adults"] = event.Props_.AdultContent_;
			map ["comment"] = event.Props_.CommentsManagement_;
			map ["hidecomment"] = event.Props_.ScreeningComments_;
			map ["place"] = event.Props_.CurrentLocation_;
			map ["music"] = event.Props_.CurrentMusic_;
			map ["moodId"] = event.Props_.CurrentMoodId_;
			map ["mood"] = event.Props_.CurrentMood_;
			map ["showInFriendsPage"] = event.Props_.ShowInFriendsPage_;
			map ["avatar"] = event.Props_.PostAvatar_;

			return map;
		}

		Event LJEvent2Event (const LJEvent& ljEvent, const QString& login)
		{
			Event event;
			event.EntryId_ = ljEvent.ItemID_;
			event.Content_ = ljEvent.Event_;
			event.Date_ = ljEvent.DateTime_;
			event.Subject_ = ljEvent.Subject_;
			event.Tags_ = ljEvent.Tags_;
			event.Target_ = login;
			event.PostOptions_ = GetPostOptionsMapFromLJEvent (ljEvent);
			return event;
		}
	}

	void LJAccount::handleGotEntries2Backup (const QList<LJEvent>& ljEvents)
	{
		QList<Event> events;
		for (const auto& ljEvent : ljEvents)
			events << LJEvent2Event (ljEvent, Login_);

		emit gotEvents2Backup (events);
	}

	void LJAccount::handleGettingEvents2BackupFinished ()
	{
		emit gettingEvents2BackupFinished ();
	}

	void LJAccount::handleGotEntries (const QList<LJEvent>& ljEvents)
	{
		QList<Event> events;
		for (const auto& ljEvent : ljEvents)
			events << LJEvent2Event (ljEvent, Login_);

		emit gotEvents (events);
	}

}
}
}
