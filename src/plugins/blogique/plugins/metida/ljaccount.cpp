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
#include "updatetypedialog.h"

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
	, IsValid_ (false)
	, LJProfile_ (std::make_shared<LJProfile> (this))
	, LoadLastEvents_ (new QAction (tr ("Last entries"), this))
	, LoadChangedEvents_ (new QAction (tr ("Changed entries"), this))
	{
		qRegisterMetaType<LJProfileData> ("LJProfileData");
		qRegisterMetaTypeStreamOperators<QList<LJFriendGroup>> ("QList<LJFriendGroup>");
		qRegisterMetaTypeStreamOperators<QList<LJMood>> ("QList<LJMood>");

		connect (LJXmlRpc_,
				SIGNAL (validatingFinished (bool)),
				this,
				SLOT (handleValidatingFinished (bool)));
		connect (LJXmlRpc_,
				SIGNAL (error (int, QString)),
				this,
				SLOT (handleXmlRpcError (int, QString)));
		connect (LJXmlRpc_,
				SIGNAL (networkError (int, QString)),
				this,
				SLOT (handleNetworkError (int, QString)));
		connect (LJXmlRpc_,
				SIGNAL (profileUpdated (LJProfileData)),
				LJProfile_.get (),
				SLOT (handleProfileUpdate (LJProfileData)));
		connect (LJXmlRpc_,
				SIGNAL (eventPosted (QList<LJEvent>)),
				this,
				SLOT (handleEventPosted (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (eventRemoved (int)),
				this,
				SIGNAL (entryRemoved (int)));
		connect (LJXmlRpc_,
				SIGNAL (eventUpdated (QList<LJEvent>)),
				this,
				SLOT (handleEventUpdated (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gotEvents2Backup (QList<LJEvent>)),
				this,
				SLOT (handleGotEvents2Backup (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gettingEvents2BackupFinished ()),
				this,
				SLOT (handleGettingEvents2BackupFinished ()));
		connect (LJXmlRpc_,
				SIGNAL (gotEvents (QList<LJEvent>)),
				this,
				SLOT (handleGotEvents (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gotStatistics (QMap<QDate, int>)),
				this,
				SIGNAL (gotBlogStatistics (QMap<QDate, int>)));

		connect (LoadLastEvents_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoadLastEvents ()));
		connect (LoadChangedEvents_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoadChangedEvents ()));
	}

	QObject* LJAccount::GetQObject ()
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

	bool LJAccount::IsValid () const
	{
		return IsValid_;
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

	void LJAccount::GetEntriesByDate (const QDate& date)
	{
		LJXmlRpc_->GetEventsByDate (date);
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

		LJEvent Entry2LJEvent (const Entry& entry)
		{
			LJEvent ljEvent;
			ljEvent.ItemID_ = entry.EntryId_;
			ljEvent.Event_ = entry.Content_;
			ljEvent.DateTime_ = entry.Date_;
			ljEvent.Subject_ = entry.Subject_;
			ljEvent.Tags_ = entry.Tags_;
			ljEvent.AllowMask_ = entry.PostOptions_ ["allowMask"].toUInt ();
			ljEvent.Security_ = static_cast<Access> (entry.PostOptions_ ["access"].toInt ());
			ljEvent.Props_ = GetLJEventPropetriesFromMap (entry.PostOptions_);

			return ljEvent;
		}

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

		Entry LJEvent2Entry (const LJEvent& ljEvent, const QString& login)
		{
			Entry entry;
			entry.EntryId_ = ljEvent.ItemID_;
			entry.Content_ = ljEvent.Event_;
			entry.Date_ = ljEvent.DateTime_;
			entry.Subject_ = ljEvent.Subject_;
			entry.Tags_ = ljEvent.Tags_;
			entry.Target_ = login;
			entry.EntryUrl_ = ljEvent.Url_;
			entry.PostOptions_ = GetPostOptionsMapFromLJEvent (ljEvent);
			return entry;
		}
	}

	void LJAccount::RemoveEntry (const Entry& entry)
	{
		LJXmlRpc_->RemoveEvent (Entry2LJEvent (entry));
	}

	void LJAccount::UpdateEntry (const Entry& entry)
	{
		LJXmlRpc_->UpdateEvent (Entry2LJEvent (entry));
	}

	void LJAccount::RequestStatistics ()
	{
		LJXmlRpc_->RequestStatistics ();
	}

	QList<QAction*> LJAccount::GetUpdateActions () const
	{
		return { LoadLastEvents_, LoadChangedEvents_ };
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
					<< IsValid_
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
				>> result->IsValid_;

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
		IsValid_ = success;
		qDebug () << Q_FUNC_INFO
				<< "account"
				<< GetAccountID ()
				<< "validating result is"
				<< IsValid_;

		emit accountValidated (IsValid_);
		emit accountSettingsChanged ();
	}

	void LJAccount::handleXmlRpcError (int errorCode, const QString& msgInEng)
	{
		qWarning () << Q_FUNC_INFO
				<< "error code:"
				<< errorCode
				<< "error text:"
				<< msgInEng;

		Core::Instance ().SendEntity (Util::MakeNotification ("Blogique",
				tr ("%1 (original message: %2)")
						.arg (MetidaUtils::GetLocalizedErrorMessage (errorCode),
						msgInEng),
				PWarning_));
	}

	void LJAccount::handleNetworkError (int errorCode, const QString& msgInEng)
	{
		qWarning () << Q_FUNC_INFO
				<< "error code:"
				<< errorCode
				<< "error text:"
				<< msgInEng;

		Core::Instance ().SendEntity (Util::MakeNotification ("Blogique",
				tr ("%1 (error code: %2)")
					.arg (msgInEng)
					.arg (errorCode),
				PWarning_));
	}

	void LJAccount::updateProfile ()
	{
		LJXmlRpc_->UpdateProfileInfo ();
	}

	void LJAccount::submit (const Entry& entry)
	{
		LJEvent ljEvent;
		LJEventProperties props;

		const QVariantMap& postOptions = entry.PostOptions_;

		ljEvent.Subject_ = entry.Subject_;
		ljEvent.Event_ = entry.Content_;
		ljEvent.UseJournal_ = entry.Target_;
		ljEvent.DateTime_ = entry.Date_;
		ljEvent.Tags_ = entry.Tags_;

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

		props.CurrentMoodId_ = postOptions.value ("moodId", -1).toInt ();
		if (props.CurrentMoodId_ == -1)
			props.CurrentMood_ = postOptions.value ("mood").toString ();

		props.ShowInFriendsPage_ = postOptions.value ("showInFriendsPage").toBool ();

		props.PostAvatar_ = postOptions.value ("avatar").toString ();

		ljEvent.Props_ = props;
		ljEvent.Event_.append ("<em style=\"font-size: 0.8em;\">Posted via <a href=\"http://leechcraft.org/plugins-blogique\">LeechCraft Blogique</a>.</em>");

		LJXmlRpc_->Submit (ljEvent);
	}

	void LJAccount::backup ()
	{
		LJXmlRpc_->BackupEvents ();
	}

	void LJAccount::handleEventPosted (const QList<LJEvent>& events)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : events)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit entryPosted (entries);
	}

	void LJAccount::handleEventUpdated (const QList<LJEvent>& events)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : events)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit entryUpdated (entries);
	}

	void LJAccount::handleGotEvents2Backup (const QList<LJEvent>& ljEvents)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : ljEvents)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit gotEntries2Backup (entries);
	}

	void LJAccount::handleGettingEvents2BackupFinished ()
	{
		emit gettingEntries2BackupFinished ();
	}

	void LJAccount::handleGotEvents (const QList<LJEvent>& ljEvents)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : ljEvents)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit gotEntries (entries);
	}

	void LJAccount::handleLoadLastEvents ()
	{
		int count = 0;
		if (XmlSettingsManager::Instance ().Property ("LoadLastAsk", true).toBool ())
		{
			UpdateTypeDialog dlg (UpdateTypeDialog::LoadType::LoadLastEvents);
			if (dlg.exec () == QDialog::Rejected)
				return;
			count = dlg.GetCount ();
		}

		LJXmlRpc_->GetLastEvents (count);
	}

	void LJAccount::handleLoadChangedEvents ()
	{
		QDateTime dt;
		if (XmlSettingsManager::Instance ().Property ("LoadChangedAsk", true).toBool ())
		{
			UpdateTypeDialog dlg (UpdateTypeDialog::LoadType::LoadChangesEvents);
			if (dlg.exec () == QDialog::Rejected)
				return;
			dt = dlg.GetDateTime ();
		}

		LJXmlRpc_->GetChangedEvents (dt);
	}

}
}
}
