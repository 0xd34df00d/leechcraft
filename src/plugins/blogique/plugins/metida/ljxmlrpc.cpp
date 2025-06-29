/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ljxmlrpc.h"
#include <QDomDocument>
#include <QtDebug>
#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <util/sys/sysinfo.h>
#include <util/xpc/util.h>
#include <util/sll/debugprinters.h>
#include <util/sll/qtutil.h>
#include <util/sll/urloperator.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/domchildrenrange.h>
#include <util/svcauth/ljutils.h>
#include <util/threads/coro.h>
#include "profiletypes.h"
#include "ljfriendentry.h"
#include "utils.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	constexpr int BitMaskForFriendsOnlyComments = 0;
	constexpr int MaxGetEventsCount = 50;

	LJXmlRPC::LJXmlRPC (LJAccount *acc, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Account_ { acc }
	, Proxy_ { proxy }
	{
	}

	void LJXmlRPC::Validate (const QString& login, const QString& password)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [login, password, this] (const QString& challenge)
				{ ValidateAccountData (login, password, challenge); };
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [login, password, this] (const QString& challenge)
				{ RequestFriendsInfo (login, password, challenge); };
	}

	void LJXmlRPC::AddNewFriend (const QString& username,
			const QString& bgcolor, const QString& fgcolor, uint groupMask)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [username, bgcolor, fgcolor, groupMask, this] (const QString& challenge)
				{ AddNewFriendRequest (username, bgcolor, fgcolor, groupMask, challenge); };
	}

	void LJXmlRPC::DeleteFriend (const QString& username)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [username, this] (const QString& challenge)
				{ DeleteFriendRequest (username, challenge); };
	}

	void LJXmlRPC::AddGroup (const QString& name, bool isPublic, int id)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [name, isPublic, id, this] (const QString& challenge)
				{ AddGroupRequest (name, isPublic, id, challenge); };
	}

	void LJXmlRPC::DeleteGroup (int id)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [id, this] (const QString& challenge)
				{ DeleteGroupRequest (id, challenge); };
	}

	void LJXmlRPC::UpdateProfileInfo ()
	{
		Validate (Account_->GetOurLogin (), Account_->GetPassword ());
	}

	void LJXmlRPC::Preview (const LJEvent& event)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [event, this] (const QString& challenge)
				{ PreviewEventRequest (event, challenge); };
	}

	void LJXmlRPC::Submit (const LJEvent& event)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [event, this] (const QString& challenge)
				{ PostEventRequest (event, challenge); };
	}

	void LJXmlRPC::GetEventsWithFilter (const Filter& filter)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [filter, this] (const QString& challenge)
				{ BackupEventsRequest (filter, challenge); };
	}

	void LJXmlRPC::GetLastEvents (int count)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [count, this] (const QString& challenge)
				{ GetLastEventsRequest (count, challenge); };
	}

	void LJXmlRPC::GetMultiplyEvents (const QList<int>& ids, LJXmlRPC::RequestType rt)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [ids, rt, this] (const QString& challenge)
				{ GetMultipleEventsRequest (ids, rt, challenge); };
	}

	void LJXmlRPC::GetParticularEvent (int id, LJXmlRPC::RequestType rt)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [id, rt, this] (const QString& challenge)
				{ GetParticularEventRequest (id, rt, challenge); };
	}

	void LJXmlRPC::GetChangedEvents (const QDateTime& dt)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [dt, this] (const QString& challenge)
				{ GetChangedEventsRequest (dt, challenge); };
	}

	void LJXmlRPC::GetEventsByDate (const QDate& date, int skip)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [date, skip, this] (const QString& challenge)
				{ GetEventsByDateRequest (date, skip, challenge); };
	}

	void LJXmlRPC::RemoveEvent (const LJEvent& event)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [event, this] (const QString& challenge)
				{ RemoveEventRequest (event, challenge); };
	}

	void LJXmlRPC::UpdateEvent (const LJEvent& event)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [event, this] (const QString& challenge)
				{ UpdateEventRequest (event, challenge); };
	}

	void LJXmlRPC::RequestStatistics ()
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this] (const QString& challenge)
				{ BlogStatisticsRequest (challenge); };
	}

	void LJXmlRPC::RequestLastInbox ()
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this] (const QString& challenge)
				{ InboxRequest (challenge); };
	}

	void LJXmlRPC::SetMessagesAsRead (const QList<int>& ids)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this, ids] (const QString& challenge)
				{ SetMessageAsReadRequest (ids, challenge); };
	}

	void LJXmlRPC::SendMessage (const QStringList& addresses, const QString& subject,
			const QString& text)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this, addresses, subject, text] (const QString& challenge)
				{ SendMessageRequest (addresses, subject, text, challenge); };
	}

	void LJXmlRPC::RequestRecentCommments ()
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this] (const QString& challenge)
				{ RecentCommentsRequest (challenge); };
	}

	void LJXmlRPC::DeleteComment (qint64 id, bool deleteThread)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this, id, deleteThread] (const QString& challenge)
				{ DeleteCommentRequest (id, deleteThread, challenge); };
	}

	void LJXmlRPC::AddComment (const CommentEntry& comment)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this, comment] (const QString& challenge)
				{ AddCommentRequest (comment, challenge); };
	}

	void LJXmlRPC::RequestTags ()
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] (const QString&) { GenerateChallenge (); };
		ApiCallQueue_ << [this] (const QString& challenge)
				{ GetUserTagsRequest (challenge); };
	}

	std::shared_ptr<void> LJXmlRPC::MakeRunnerGuard ()
	{
		const bool shouldRun = ApiCallQueue_.isEmpty ();
		return std::shared_ptr<void> (nullptr, [this, shouldRun] (void*)
				{
					if (shouldRun)
						ApiCallQueue_.dequeue () (QString ());
				});
	}

	void LJXmlRPC::CallNextFunctionFromQueue ()
	{
		if (!ApiCallQueue_.isEmpty () && !(ApiCallQueue_.count () % 2))
			ApiCallQueue_.dequeue () (QString ());
	}

	namespace
	{
		QPair<QDomElement, QDomElement> GetStartPart (const QString& name,
				QDomDocument doc)
		{
			QDomElement methodCall = doc.createElement ("methodCall");
			doc.appendChild (methodCall);
			QDomElement methodName = doc.createElement ("methodName");
			methodCall.appendChild (methodName);
			QDomText methodNameText = doc.createTextNode (name);
			methodName.appendChild (methodNameText);
			QDomElement params = doc.createElement ("params");
			methodCall.appendChild (params);
			QDomElement param = doc.createElement ("param");
			params.appendChild (param);
			QDomElement value = doc.createElement ("value");
			param.appendChild (value);
			QDomElement structElem = doc.createElement ("struct");
			value.appendChild (structElem);

			return {methodCall, structElem};
		}

		QDomElement GetSimpleMemberElement (const QString& nameVal,
				const QString& typeVal, const QString& value, QDomDocument doc)
		{
			QDomElement member = doc.createElement ("member");
			QDomElement name = doc.createElement ("name");
			member.appendChild (name);
			QDomText nameText = doc.createTextNode (nameVal);
			name.appendChild (nameText);
			QDomElement valueType = doc.createElement ("value");
			member.appendChild (valueType);
			QDomElement type = doc.createElement (typeVal);
			valueType.appendChild (type);
			QDomText text = doc.createTextNode (value);
			type.appendChild (text);

			return member;
		}

		QPair<QDomElement, QDomElement> GetComplexMemberElement (const QString& nameVal,
				const QString& typeVal, QDomDocument doc)
		{
			QDomElement member = doc.createElement ("member");
			QDomElement name = doc.createElement ("name");
			member.appendChild (name);
			QDomText nameText = doc.createTextNode (nameVal);
			name.appendChild (nameText);
			QDomElement valueType = doc.createElement ("value");
			member.appendChild (valueType);
			QDomElement type = doc.createElement (typeVal);
			valueType.appendChild (type);
			QDomElement dataField;
			if (typeVal == "array")
			{
				dataField = doc.createElement ("data");
				type.appendChild (dataField);
			}
			else if (typeVal == "struct")
				dataField = type;

			return { member, dataField };
		}

		QNetworkRequest CreateNetworkRequest (const ICoreProxy_ptr& proxy)
		{
			QNetworkRequest request;
			auto userAgent = "LeechCraft Blogique " + proxy->GetVersion ().toUtf8 ();
			request.setUrl (QUrl ("http://www.livejournal.com/interface/xmlrpc"));
			request.setRawHeader ("User-Agent", userAgent);
			request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");

			return request;
		}

		QNetworkReply* Post (const ICoreProxy_ptr& proxy, const QDomDocument& document)
		{
			return proxy->GetNetworkAccessManager ()->post (CreateNetworkRequest (proxy), document.toByteArray ());
		}

		QString GetPassword (const QString& password, const QString& challenge)
		{
			const QByteArray passwordHash = QCryptographicHash::hash (password.toUtf8 (),
					QCryptographicHash::Md5).toHex ();
			return QCryptographicHash::hash ((challenge + passwordHash).toUtf8 (),
					QCryptographicHash::Md5).toHex ();
		}

		QDomElement FillServicePart (QDomElement parentElement,
				const QString& login, const QString& password,
				const QString& challenge, QDomDocument document)
		{
			parentElement.appendChild (GetSimpleMemberElement ("auth_method", "string",
					"challenge", document));
			parentElement.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
					challenge, document));
			parentElement.appendChild (GetSimpleMemberElement ("username", "string",
					login, document));
			parentElement.appendChild (GetSimpleMemberElement ("auth_response", "string",
					GetPassword (password, challenge), document));
			parentElement.appendChild (GetSimpleMemberElement ("ver", "int",
					"1", document));

			return parentElement;
		}
	}

	Util::ContextTask<> LJXmlRPC::GenerateChallenge ()
	{
		co_await Util::AddContextObject { *this };

		const auto result = co_await Util::LJ::RequestChallenge ({
					.NAM_ = *GetProxyHolder ()->GetNetworkAccessManager (),
					.UserAgent_ = "LeechCraft Blogique " + GetProxyHolder ()->GetVersion ().toUtf8 (),
				});
		const auto challenge = co_await Util::WithHandler (result,
				[] (const auto& error)
				{
					const auto e = Util::MakeNotification ("Blogique",
						tr ("Unable to get authentication challenge.") + ' ' + error.Text_,
						Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});

		if (!ApiCallQueue_.isEmpty ())
			ApiCallQueue_.dequeue () (challenge);
	}

	void LJXmlRPC::ValidateAccountData (const QString& login,
			const QString& password, const QString& challenge)
	{
		QDomDocument document ("ValidateRequest");
		auto result = GetStartPart ("LJ.XMLRPC.login", document);
		document.appendChild (result.first);

		auto element = FillServicePart (result.second,
				login, password, challenge, document);
		element.appendChild (GetSimpleMemberElement ("clientversion", "string",
				Util::SysInfo::GetOSName () +
					"-LeechCraft Blogique: " +
					Proxy_->GetVersion (),
				document));
		element.appendChild (GetSimpleMemberElement ("getmoods", "int",
				"0", document));
		element.appendChild (GetSimpleMemberElement ("getmenus", "int",
				"0", document));
		element.appendChild (GetSimpleMemberElement ("getpickws", "int",
				"1", document));
		element.appendChild (GetSimpleMemberElement ("getpickwurls", "int",
				"1", document));
		element.appendChild (GetSimpleMemberElement ("getcaps", "int",
				"1", document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleValidateReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::RequestFriendsInfo (const QString& login,
			const QString& password, const QString& challenge)
	{
		QDomDocument document ("GetFriendsInfo");
		auto result = GetStartPart ("LJ.XMLRPC.getfriends", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second,
				login, password, challenge, document);
		element.appendChild (GetSimpleMemberElement ("includebdays", "boolean",
				"1", document));
		element.appendChild (GetSimpleMemberElement ("includefriendof", "boolean",
				"1", document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFriendsInfoFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::AddNewFriendRequest (const QString& username,
			const QString& bgcolor, const QString& fgcolor,
			int groupMask, const QString& challenge)
	{
		QDomDocument document ("AddNewFriendRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriends", document);
		document.appendChild (result.first);

		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		auto array = GetComplexMemberElement ("add", "array", document);
 		element.appendChild (array.first);
		auto structField = document.createElement ("struct");
		array.second.appendChild (structField);

		structField.appendChild (GetSimpleMemberElement ("username", "string",
				username, document));
		if (!fgcolor.isEmpty ())
			structField.appendChild (GetSimpleMemberElement ("fgcolor", "string",
					fgcolor, document));
		if (!bgcolor.isEmpty ())
			structField.appendChild (GetSimpleMemberElement ("bgcolor", "string",
					bgcolor, document));
		structField.appendChild (GetSimpleMemberElement ("groupmask", "int",
				QString::number (groupMask), document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAddNewFriendReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::DeleteFriendRequest (const QString& username, const QString& challenge)
	{
		QDomDocument document ("DeleteFriendRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriends", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		auto array = GetComplexMemberElement ("delete", "array", document);
		element.appendChild (array.first);
		QDomElement valueField = document.createElement ("value");
		array.second.appendChild (valueField);
		QDomElement valueType = document.createElement ("string");
		valueField.appendChild (valueType);
		QDomText text = document.createTextNode (username);
		valueType.appendChild (text);

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyWithProfileUpdate ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::AddGroupRequest (const QString& name, bool isPublic, int id,
			const QString& challenge)
	{
		QDomDocument document ("AddNewFriendRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriendgroups", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		auto data = GetComplexMemberElement ("set", "struct", document);
		element.appendChild (data.first);
		auto subStruct = GetComplexMemberElement (QString::number (id), "struct", document);
		data.second.appendChild (subStruct.first);
		subStruct.second.appendChild (GetSimpleMemberElement ("name", "string",
				name, document));
		subStruct.second.appendChild (GetSimpleMemberElement ("public", "boolean",
				 isPublic ? "1" : "0", document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyWithProfileUpdate ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::DeleteGroupRequest (int id, const QString& challenge)
	{
		QDomDocument document ("DeleteGroupRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriendgroups", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		auto array = GetComplexMemberElement ("delete", "array", document);
		element.appendChild (array.first);
		QDomElement valueField = document.createElement ("value");
		array.second.appendChild (valueField);
		QDomElement valueType = document.createElement ("int");
		valueField.appendChild (valueType);
		QDomText text = document.createTextNode (QString::number (id));
		valueType.appendChild (text);

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyWithProfileUpdate ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::PreviewEventRequest (const LJEvent& event, const QString& challenge)
	{
		QNetworkRequest request (QUrl ("http://www.livejournal.com/preview/entry.bml"));
		auto userAgent = "LeechCraft Blogique " + Proxy_->GetVersion ().toUtf8 ();
		request.setRawHeader ("User-Agent", userAgent);
		request.setRawHeader ("Referer", "http://www.livejournal.com/update.bml");
		request.setRawHeader ("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		request.setRawHeader ("Accept-Language", "en-US,en;q=0.5");
		request.setRawHeader ("Accept-Encoding", "gzip, deflate");
		request.setRawHeader ("Referer", "http://www.livejournal.com/update.bml");
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		QUrl params;
		Util::UrlOperator { params }
				("lj_form_auth", challenge)
				("rte_on", "1")
				("date_diff", "1")
				("date_format", "%M/%D/%Y")
				("postas", "remote")
				("postto", "journal")
				("community", "books_dot_ru")
				("altcommunity", "")
				("date", event.DateTime_.toString ("dd/MM/yyyy"))
				("time", event.DateTime_.toString ("hh:mm"))
				("custom_time", "0")
				("timezone", "300")
				("prop_picture_keyword", event.Props_.PostAvatar_)
				("subject", event.Subject_)
				("event", event.Event_)
				("prop_taglist", event.Tags_.join (","))
				("prop_current_moodid", QString::number (event.Props_.CurrentMoodId_))
				("prop_current_mood", event.Props_.CurrentMood_)
				("prop_current_music", event.Props_.CurrentMusic_)
				("prop_current_location", event.Props_.CurrentLocation_)
				("prop_adult_conten", MetidaUtils::GetStringForAdultContent (event.Props_.AdultContent_))
				("comment_settings", "default")
				("prop_opt_screening", "")
				("security", MetidaUtils::GetStringForAccess (event.Security_))
				("date_ymd_dd", QString::number (event.DateTime_.date ().day ()))
				("date_ymd_mm", QString::number (event.DateTime_.date ().month ()))
				("date_ymd_yyyy", QString::number (event.DateTime_.date ().year ()))
				("date_diff", "1");

		const auto& payload = QUrlQuery { params }.toString (QUrl::FullyEncoded).toUtf8 ();
		auto reply = Proxy_->GetNetworkAccessManager ()->post (request, payload);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handlePreviewEventReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::PostEventRequest (const LJEvent& event, const QString& challenge)
	{
		QDomDocument document ("PostEventRequest");
		auto result = GetStartPart ("LJ.XMLRPC.postevent", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("event", "string",
				event.Event_, document));
		element.appendChild (GetSimpleMemberElement ("subject", "string",
				event.Subject_, document));
		element.appendChild (GetSimpleMemberElement ("security", "string",
				MetidaUtils::GetStringForAccess (event.Security_), document));
		if (event.Security_ == Access::FriendsOnly)
			element.appendChild (GetSimpleMemberElement ("allowmask", "int",
					QString::number (BitMaskForFriendsOnlyComments), document));
		else if (event.Security_ == Access::Custom)
			element.appendChild (GetSimpleMemberElement ("allowmask", "int",
					QString::number (event.AllowMask_), document));
		element.appendChild (GetSimpleMemberElement ("year", "int",
				QString::number (event.DateTime_.date ().year ()), document));
		element.appendChild (GetSimpleMemberElement ("mon", "int",
				QString::number (event.DateTime_.date ().month ()), document));
		element.appendChild (GetSimpleMemberElement ("day", "int",
				QString::number (event.DateTime_.date ().day ()), document));
		element.appendChild (GetSimpleMemberElement ("hour", "int",
				QString::number (event.DateTime_.time ().hour ()), document));
		element.appendChild (GetSimpleMemberElement ("min", "int",
				QString::number (event.DateTime_.time ().minute ()), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				event.UseJournal_, document));

		auto propsStruct = GetComplexMemberElement ("props", "struct", document);
		element.appendChild (propsStruct.first);
		propsStruct.second.appendChild (GetSimpleMemberElement ("current_location",
				"string", event.Props_.CurrentLocation_, document));
		if (event.Props_.CurrentMoodId_ == -1)
			propsStruct.second.appendChild (GetSimpleMemberElement ("current_mood",
					"string", event.Props_.CurrentMood_, document));
		else
			propsStruct.second.appendChild (GetSimpleMemberElement ("current_moodid",
					"int", QString::number (event.Props_.CurrentMoodId_), document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("current_music",
				"string", event.Props_.CurrentMusic_, document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("opt_nocomments",
				"boolean", event.Props_.CommentsManagement_ == CommentsManagement::DisableComments ?
					"1" :
					"0",
				document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("opt_noemail",
				"boolean", event.Props_.CommentsManagement_ == CommentsManagement::WithoutNotification ?
					"1" :
					"0",
				document));
		QString screening = MetidaUtils::GetStringFromCommentsManagment (event.Props_.CommentsManagement_);
		if (!screening.isEmpty ())
			propsStruct.second.appendChild (GetSimpleMemberElement ("opt_screening",
					"string", screening, document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("adult_content",
				"string",
				MetidaUtils::GetStringForAdultContent (event.Props_.AdultContent_),
				document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("taglist",
				"string", event.Tags_.join (","), document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("useragent",
				"string", "LeechCraft Blogique", document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("picture_keyword",
				"string", event.Props_.PostAvatar_, document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handlePostEventReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::RemoveEventRequest (const LJEvent& event, const QString& challenge)
	{
		QDomDocument document ("RemoveEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editevent", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("itemid", "int",
				QString::number (event.ItemID_), document));
		element.appendChild (GetSimpleMemberElement ("event", "string",
				QString (), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRemoveEventReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::UpdateEventRequest (const LJEvent& event, const QString& challenge)
	{
		QDomDocument document ("EditEventRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editevent", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("itemid", "int",
				QString::number (event.ItemID_), document));
		element.appendChild (GetSimpleMemberElement ("event", "string",
				event.Event_, document));
		element.appendChild (GetSimpleMemberElement ("subject", "string",
				event.Subject_, document));

		element.appendChild (GetSimpleMemberElement ("security", "string",
				MetidaUtils::GetStringForAccess (event.Security_), document));
		if (event.Security_ == Access::FriendsOnly)
			element.appendChild (GetSimpleMemberElement ("allowmask", "int",
					QString::number (BitMaskForFriendsOnlyComments), document));
		else if (event.Security_ == Access::Custom)
			element.appendChild (GetSimpleMemberElement ("allowmask", "int",
					QString::number (event.AllowMask_), document));
		element.appendChild (GetSimpleMemberElement ("year", "int",
				QString::number (event.DateTime_.date ().year ()), document));
		element.appendChild (GetSimpleMemberElement ("mon", "int",
				QString::number (event.DateTime_.date ().month ()), document));
		element.appendChild (GetSimpleMemberElement ("day", "int",
				QString::number (event.DateTime_.date ().day ()), document));
		element.appendChild (GetSimpleMemberElement ("hour", "int",
				QString::number (event.DateTime_.time ().hour ()), document));
		element.appendChild (GetSimpleMemberElement ("min", "int",
				QString::number (event.DateTime_.time ().minute ()), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto propsStruct = GetComplexMemberElement ("props", "struct", document);
		element.appendChild (propsStruct.first);
		propsStruct.second.appendChild (GetSimpleMemberElement ("current_location",
				"string", event.Props_.CurrentLocation_, document));
		if (event.Props_.CurrentMoodId_ == -1)
			propsStruct.second.appendChild (GetSimpleMemberElement ("current_mood",
					"string", event.Props_.CurrentMood_, document));
		else
			propsStruct.second.appendChild (GetSimpleMemberElement ("current_moodid",
					"int", QString::number (event.Props_.CurrentMoodId_), document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("current_music",
				"string", event.Props_.CurrentMusic_, document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("opt_nocomments",
				"boolean", event.Props_.CommentsManagement_ == CommentsManagement::DisableComments ?
					"1" :
					"0",
				document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("opt_noemail",
				"boolean", event.Props_.CommentsManagement_ == CommentsManagement::WithoutNotification ?
					"1" :
					"0",
				document));
		QString screening = MetidaUtils::GetStringFromCommentsManagment (event.Props_.CommentsManagement_);
		if (!screening.isEmpty ())
			propsStruct.second.appendChild (GetSimpleMemberElement ("opt_screening",
					"string", screening, document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("adult_content",
				"string",
				MetidaUtils::GetStringForAdultContent (event.Props_.AdultContent_),
				document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("taglist",
				"string", event.Tags_.join (","), document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("useragent",
				"string", "LeechCraft Blogique", document));
		propsStruct.second.appendChild (GetSimpleMemberElement ("picture_keyword",
				"string", event.Props_.PostAvatar_, document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUpdateEventReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::BackupEventsRequest (const Filter& filter,
			const QString& challenge)
	{
		QDomDocument document ("BackupEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getevents", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("selecttype", "string",
				"before", document));
		element.appendChild (GetSimpleMemberElement ("before", "string",
				(!filter.CustomDate_ ? QDateTime::currentDateTime () : filter.EndDate_)
						.toString ("yyyy-MM-dd hh:MM:ss"),
				document));
		element.appendChild (GetSimpleMemberElement ("howmany", "int",
				QString::number (MaxGetEventsCount), document));
		element.appendChild (GetSimpleMemberElement ("skip", "int",
				QString::number (filter.Skip_), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		Reply2Filter_ [reply] = filter;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleBackupEventsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::GetLastEventsRequest (int count, const QString& challenge)
	{
		QDomDocument document ("GetLastEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getevents", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("selecttype", "string",
				"lastn", document));
		element.appendChild (GetSimpleMemberElement ("howmany", "int",
				QString::number (count), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));
		// for debug lj-tags
// 		element.appendChild (GetSimpleMemberElement ("parseljtags", "boolean",
// 				"1", document));
		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotEventsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::GetChangedEventsRequest (const QDateTime& dt, const QString& challenge)
	{
		QDomDocument document ("GetLastEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getevents", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("selecttype", "string",
				"syncitems", document));
		element.appendChild (GetSimpleMemberElement ("lastsync", "string",
				dt.toString ("yyyy-MM-dd hh:mm:ss"), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotEventsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::GetEventsByDateRequest (const QDate& date, int skip, const QString& challenge)
	{
		QDomDocument document ("GetLastEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getevents", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("selecttype", "string",
				"day", document));

		element.appendChild (GetSimpleMemberElement ("day", "int",
				QString::number (date.day ()), document));
		element.appendChild (GetSimpleMemberElement ("month", "int",
				QString::number (date.month ()), document));
		element.appendChild (GetSimpleMemberElement ("year", "int",
				QString::number (date.year ()), document));
		element.appendChild (GetSimpleMemberElement ("skip", "int",
				QString::number (skip), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);

		Reply2Skip_ [reply] = skip;
		Reply2Date_ [reply] = date;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotEventsByDateReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::GetParticularEventRequest (int id, RequestType prt,
			const QString& challenge)
	{
		QDomDocument document ("GetParticularEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getevents", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("selecttype", "string",
				"one", document));
		element.appendChild (GetSimpleMemberElement ("itemid", "int",
				QString::number (id), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		Reply2RequestType_ [reply] = prt;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetParticularEventReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::GetMultipleEventsRequest (const QList<int>& ids, RequestType rt,
			const QString& challenge)
	{
		QStringList list;
		for (int id : ids)
			list << QString::number (id);

		QDomDocument document ("GetParticularEventsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getevents", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("prefersubject", "boolean",
				"true", document));
		element.appendChild (GetSimpleMemberElement ("noprops", "boolean",
				"true", document));
		element.appendChild (GetSimpleMemberElement ("notags", "boolean",
				"true", document));
		element.appendChild (GetSimpleMemberElement ("selecttype", "string",
				"multiple", document));
		element.appendChild (GetSimpleMemberElement ("itemids", "int",
				list.join (","), document));
		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		Reply2RequestType_ [reply] = rt;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetMultipleEventsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::BlogStatisticsRequest (const QString& challenge)
	{
		QDomDocument document ("BlogStatisticsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getdaycounts", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleBlogStatisticsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::InboxRequest (const QString& challenge)
	{
		QDomDocument document ("InboxRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getinbox", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);
		const auto& lastMonth = QDate::currentDate ().addMonths (-1).startOfDay ();
		const uint lastSyncDate = XmlSettingsManager::Instance ().Property ("LastInboxUpdateDate", lastMonth)
				.toDateTime ().toSecsSinceEpoch ();
		element.appendChild (GetSimpleMemberElement ("lastsync",
				"string",
				QString::number (lastSyncDate),
				document));

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleInboxReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::SetMessageAsReadRequest (const QList<int>& ids, const QString& challenge)
	{
		QDomDocument document ("SetMessageAsReadRequest");
		auto result = GetStartPart ("LJ.XMLRPC.setmessageread", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		auto array = GetComplexMemberElement ("qid", "array", document);
 		element.appendChild (array.first);

		for (int id : ids)
		{
			QDomElement valueType = document.createElement ("value");
			array.second.appendChild (valueType);
			QDomElement type = document.createElement ("int");
			valueType.appendChild (type);
			QDomText text = document.createTextNode (QString::number (id));
			type.appendChild (text);
		}

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleMessagesSetAsReadFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::SendMessageRequest (const QStringList& addresses, const QString& subject,
			const QString& text, const QString& challenge)
	{
		QDomDocument document ("SendMessageRequest");
		auto result = GetStartPart ("LJ.XMLRPC.sendmessage", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);
		element.appendChild (GetSimpleMemberElement ("subject", "string", subject, document));
		element.appendChild (GetSimpleMemberElement ("body", "string", text, document));
		auto array = GetComplexMemberElement ("to", "array", document);
		element.appendChild (array.first);
		for (const auto& address : addresses)
		{
			QDomElement valueType = document.createElement ("value");
			array.second.appendChild (valueType);
			QDomElement type = document.createElement ("string");
			valueType.appendChild (type);
			QDomText text = document.createTextNode (address);
			type.appendChild (text);
		}

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleSendMessageRequestFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::RecentCommentsRequest (const QString& challenge)
	{
		QDomDocument document ("RecentCommentsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getrecentcomments", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("itemshow", "int",
				XmlSettingsManager::Instance ().Property ("RecentCommentsNumber", 10).toString (),
				document));

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRecentCommentsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::DeleteCommentRequest (qint64 id, bool deleteThread, const QString& challenge)
	{
		QDomDocument document ("DeleteCommentRequest");
		auto result = GetStartPart ("LJ.XMLRPC.deletecomments", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);
		element.appendChild (GetSimpleMemberElement ("dtalkid", "int",
				QString::number (id), document));
		if (deleteThread)
			element.appendChild (GetSimpleMemberElement ("thread", "boolean",
					"true", document));
		element.appendChild (GetSimpleMemberElement ("journal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleDeleteCommentReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::AddCommentRequest (const CommentEntry& comment, const QString& challenge)
	{
		QDomDocument document ("AddCommentRequest");
		auto result = GetStartPart ("LJ.XMLRPC.addcomment", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);
		element.appendChild (GetSimpleMemberElement ("body", "string",
				comment.CommentText_, document));
		element.appendChild (GetSimpleMemberElement ("subject", "string",
				comment.CommentSubject_, document));
		element.appendChild (GetSimpleMemberElement ("ditemid", "string",
				QString::number (comment.EntryID_), document));
		element.appendChild (GetSimpleMemberElement ("parent", "string",
				QString::number (comment.ParentCommentID_), document));
		element.appendChild (GetSimpleMemberElement ("journal", "string",
				Account_->GetOurLogin (), document));

		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAddCommentReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::GetUserTagsRequest (const QString& challenge)
	{
		QDomDocument document ("REecentCommentsRequest");
		auto result = GetStartPart ("LJ.XMLRPC.getusertags", document);
		document.appendChild (result.first);
		auto element = FillServicePart (result.second, Account_->GetOurLogin (),
				Account_->GetPassword (), challenge, document);

		element.appendChild (GetSimpleMemberElement ("usejournal", "string",
				Account_->GetOurLogin (), document));
		auto reply = Post (Proxy_, document);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetUserTagsReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void LJXmlRPC::ParseForError (const QByteArray& content)
	{
		QDomDocument doc;
		if (!doc.setContent (content))
		{
			qWarning () << "unable to parse XML:" << content;
			return;
		}

		const auto& faultStruct = doc.documentElement ()
				.firstChildElement ("methodResponse"_qs)
				.firstChildElement ("fault"_qs)
				.firstChildElement ("value"_qs)
				.firstChildElement ("struct"_qs);
		if (faultStruct.isNull ())
			return;

		std::optional<int> errorCode;
		std::optional<QString> errorString;

		for (const auto& member : Util::DomChildren (faultStruct, "member"_qs))
		{
			const auto& name = member.firstChildElement ("name").text ();
			if (name == "faultCode"_qs)
			{
				bool ok;
				errorCode = member
						.firstChildElement ("value"_qs)
						.firstChildElement ("int"_qs)
						.text ().toInt (&ok);
				if (!ok)
					errorCode.reset ();
			}

			if (name == "faultString"_qs)
			{
				const auto& str = member
						.firstChildElement ("value"_qs)
						.firstChildElement ("string"_qs).text ();
				if (!str.isEmpty ())
					errorString = str;
			}
		}

		if (errorCode)
			emit error (*errorCode, errorString.value_or ("unknown error"_qs),
					MetidaUtils::GetLocalizedErrorMessage (*errorCode));
	}

	namespace
	{
		QVariantList ParseValue (const QDomNode& node);

		LJParserTypes::LJParseProfileEntry ParseMember (const QDomNode& node)
		{
			const auto& memberFields = node.childNodes ();
			const auto& memberNameField = memberFields.at (0);
			const auto& memberValueField = memberFields.at (1);
			QString memberName;
			QVariantList memberValue;
			if (memberNameField.isElement () &&
					memberNameField.toElement ().tagName () == "name")
				memberName = memberNameField.toElement ().text ();

			if (memberValueField.isElement ())
				memberValue = ParseValue (memberValueField);

			return LJParserTypes::LJParseProfileEntry (memberName, memberValue);
		}

		QVariantList ParseValue (const QDomNode& node)
		{
			QVariantList result;
			const auto& valueNode = node.firstChild ();
			const auto& valueElement = valueNode.toElement ();
			QString type = valueElement.tagName ();
			if (type == "string" ||
					type == "int" ||
					type == "i4" ||
					type == "double" ||
					type == "boolean")
				result << valueElement.text ();
			else if (type == "dateTime.iso8601")
				result << QDateTime::fromString (valueElement.text (), Qt::ISODate);
			else if (type == "base64")
				result << QString::fromUtf8 (QByteArray::fromBase64 (valueElement.text ().toUtf8 ()));
			else if (type == "array")
			{
				const auto& arrayElements = valueNode.firstChild ().childNodes ();
				QVariantList array;
				for (int i = 0, count = arrayElements.count (); i < count; ++i)
					array << QVariant::fromValue<QVariantList> (ParseValue (arrayElements.at (i)));

				result << array;
			}
			else if (type == "struct")
			{
				const auto& structMembers = valueNode.childNodes ();
				for (int i = 0, count = structMembers.count (); i < count; ++i)
					result << QVariant::fromValue<LJParserTypes::LJParseProfileEntry> (ParseMember (structMembers.at (i)));
			}

			return result;
		}

		void CreateFriendEntry (const QString& parentKey, const QVariantList& data, QHash<QString, LJFriendEntry_ptr>& frHash)
		{
			for (const auto& friendEntry : data)
			{
				LJFriendEntry_ptr fr = std::make_shared<LJFriendEntry> ();
				bool isCommunity = false, personal = false;

				for (const auto& field : friendEntry.toList ())
				{
					auto fieldEntry = field.value<LJParserTypes::LJParseProfileEntry> ();
					if (fieldEntry.Name () == "defaultpicurl")
						fr->SetAvatarUrl (fieldEntry.ValueToUrl ());
					else if (fieldEntry.Name () == "bgcolor")
						fr->SetBGColor (fieldEntry.ValueToString ());
					else if (fieldEntry.Name () == "fgcolor")
						fr->SetFGColor (fieldEntry.ValueToString ());
					else if (fieldEntry.Name () == "groupmask")
						fr->SetGroupMask (fieldEntry.ValueToInt ());
					else if (fieldEntry.Name () == "fullname")
						fr->SetFullName (fieldEntry.ValueToString ());
					else if (fieldEntry.Name () == "username")
						fr->SetUserName (fieldEntry.ValueToString ());
					else if (fieldEntry.Name () == "type")
						isCommunity = (fieldEntry.ValueToString () == "community");
					else if (fieldEntry.Name () == "journaltype")
					{
						isCommunity = (fieldEntry.ValueToString () == "C");
						personal = (fieldEntry.ValueToString () == "P");
					}
					else if (fieldEntry.Name () == "birthday")
						fr->SetBirthday (fieldEntry.ValueToString ());

					if (parentKey == "friends" ||
							parentKey == "added")
						fr->SetMyFriend (true);

					if (parentKey == "friendofs")
						fr->SetFriendOf (true);
				}

				if (!isCommunity ||
						personal)
				{
					if (parentKey == "friendofs" &&
							frHash.contains (fr->GetUserName ()))
						frHash [fr->GetUserName ()]->SetFriendOf (true);
					else if ((parentKey == "friends" ||
							parentKey == "added") &&
							frHash.contains (fr->GetUserName ()))
						frHash [fr->GetUserName ()]->SetMyFriend (true);
					else
						frHash [fr->GetUserName ()] = fr;
				}
			}
		}

		LJEventProperties CreateLJEventPropetries (QStringList& tags, const QVariantList& data)
		{
			LJEventProperties props;
			for (const auto& prop : data)
			{
				auto propsFieldEntry = prop.value<LJParserTypes::LJParseProfileEntry> ();
				if (propsFieldEntry.Name () == "picture_keyword")
					props.PostAvatar_ = propsFieldEntry.ValueToString ();
				else if (propsFieldEntry.Name () == "opt_screening")
					props.CommentsManagement_ = MetidaUtils::GetCommentsManagmentFromString (propsFieldEntry.ValueToString ());
				else if (propsFieldEntry.Name () == "current_music")
					props.CurrentMusic_ = propsFieldEntry.ValueToString ();
				else if (propsFieldEntry.Name () == "current_mood")
					props.CurrentMood_ = propsFieldEntry.ValueToString ();
				else if (propsFieldEntry.Name () == "current_location")
					props.CurrentLocation_ = propsFieldEntry.ValueToString ();
				else if (propsFieldEntry.Name () == "taglist")
					tags = propsFieldEntry.ValueToString ().split (", ");
				else if (propsFieldEntry.Name () == "adult_content")
					props.AdultContent_ = MetidaUtils::GetAdultContentFromString (propsFieldEntry.ValueToString ());
				else if (propsFieldEntry.Name () == "opt_nocomments")
					props.CommentsManagement_ = MetidaUtils::GetCommentsManagmentFromInt (propsFieldEntry.ValueToInt ());
				else if (propsFieldEntry.Name () == "repost")
					props.IsRepost_ = (propsFieldEntry.ValueToString () == "c");
				else if (propsFieldEntry.Name () == "repost_url")
					props.RepostUrl_ = propsFieldEntry.ValueToUrl ();
			}

			return props;
		}

		LJEvent CreateLJEvent (const QString& , const QVariant& data)
		{
			LJEvent ljEvent;
			bool repost = false;
			QUrl url;
			QUrl originUrl;
			for (const auto& field : data.toList ())
			{
				auto fieldEntry = field.value<LJParserTypes::LJParseProfileEntry> ();
				if (fieldEntry.Name () == "itemid")
					ljEvent.ItemID_ = fieldEntry.ValueToLongLong ();
				else if (fieldEntry.Name () == "subject")
					ljEvent.Subject_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "event")
					ljEvent.Event_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "ditemid")
					ljEvent.DItemID_ = fieldEntry.ValueToLongLong ();
				else if (fieldEntry.Name () == "eventtime")
					ljEvent.DateTime_ = QDateTime::fromString (fieldEntry.ValueToString (),
							"yyyy-MM-dd hh:mm:ss");
				else if (fieldEntry.Name () == "props")
				{
					QStringList tags;
					ljEvent.Props_ = CreateLJEventPropetries (tags, fieldEntry.Value ());
					ljEvent.Tags_ = tags;
				}
				else if (fieldEntry.Name () == "url")
					url = QUrl (fieldEntry.ValueToUrl ());
				else if (fieldEntry.Name () == "anum")
					ljEvent.ANum_ = fieldEntry.ValueToInt ();
				else if (fieldEntry.Name () == "security")
					ljEvent.Security_ = MetidaUtils::GetAccessForString (fieldEntry.ValueToString ());
				else if (fieldEntry.Name () == "repost" &&
						fieldEntry.ValueToInt () == 1)
					repost = true;
				else if (fieldEntry.Name () == "original_entry_url")
					originUrl = QUrl (fieldEntry.ValueToUrl ());
			}

			ljEvent.Url_ = repost ? originUrl : url;

			return ljEvent;
		}

		QMap<QDate, int> ParseStatistics (const QDomDocument& document)
		{
			QMap<QDate, int> statistics;

			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return statistics;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
					member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () == "daycounts")
					for (const auto& element : res.Value ())
					{
						int count = 0;
						QDate date;
						for (const auto& arrayElem : element.toList ())
						{
							auto entry = arrayElem.value<LJParserTypes::LJParseProfileEntry> ();
							if (entry.Name () == "count")
								count = entry.ValueToInt ();
							else if (entry.Name () == "date")
								date = QDate::fromString (entry.ValueToString (),
										"yyyy-MM-dd");
						}

						statistics [date] = count;
					}
			}

			return statistics;
		}

		QList<LJEvent> ParseFullEvents (const QString& login, const QDomDocument& document)
		{
			QList<LJEvent> events;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return events;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
					member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () == "events")
					for (const auto& event : res.Value ())
						events << CreateLJEvent (login, event);
			}

			return events;
		}
	}

	void LJXmlRPC::ParseFriends (const QDomDocument& document)
	{
		const auto& firstStructElement = document.elementsByTagName ("struct");

		if (firstStructElement.at (0).isNull ())
			return;

		const auto& members = firstStructElement.at (0).childNodes ();
		QHash<QString, LJFriendEntry_ptr> frHash;
		for (int i = 0, count = members.count (); i < count; ++i)
		{
			const QDomNode& member = members.at (i);
			if (!member.isElement () ||
					member.toElement ().tagName () != "member")
				continue;

			auto res = ParseMember (member);
			if (res.Name () == "friends" ||
					res.Name () == "added" ||
					res.Name () == "friendofs")
				CreateFriendEntry (res.Name (), res.Value (), frHash);
		}
		Account_->AddFriends (frHash.values ());
	}

	namespace
	{
		QByteArray CreateDomDocumentFromReply (QNetworkReply *reply, QDomDocument &document)
		{
			if (!reply)
				return QByteArray ();

			const auto& content = reply->readAll ();
			reply->deleteLater ();
			if (const auto result = document.setContent (content); !result)
			{
				qWarning () << "unable to parse reply" << result;
				return QByteArray ();
			}

			return content;
		}
	}

	namespace
	{
		LJFriendGroup CreateGroup (const QVariantList& data)
		{
			LJFriendGroup group;
			for (const auto& field : data)
			{
				const auto& fieldEntry = field.value<LJParserTypes::LJParseProfileEntry> ();
				if (fieldEntry.Name () == "public")
					group.Public_ = fieldEntry.ValueToBool ();
				else if (fieldEntry.Name () == "name")
					group.Name_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "id")
				{
					group.Id_ = fieldEntry.ValueToInt ();
					group.RealId_ = (1 << group.Id_) + 1;
				}
				else if (fieldEntry.Name () == "sortorder")
					group.SortOrder_ = fieldEntry.ValueToInt ();
			}

			return group;
		}

		LJMood CreateMood (const QVariantList& data)
		{
			LJMood mood;
			for (const auto& field : data)
			{
				const auto& fieldEntry = field.value<LJParserTypes::LJParseProfileEntry> ();
				if (fieldEntry.Name () == "parent")
					mood.Parent_ = fieldEntry.ValueToLongLong ();
				else if (fieldEntry.Name () == "name")
					mood.Name_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "id")
					mood.Id_ = fieldEntry.ValueToLongLong ();
			}

			return mood;
		}

		struct Id2ProfileField
		{
			QHash<QString, std::function<void (LJProfileData&,
					const LJParserTypes::LJParseProfileEntry&)>> Id2ProfileField_;

			Id2ProfileField ()
			{
				Id2ProfileField_ ["defaultpicurl"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.AvatarUrl_ = entry.ValueToUrl ();
				};
				Id2ProfileField_ ["friendgroups"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& friendGroupEntry : entry.Value ())
						profile.FriendGroups_ << CreateGroup (friendGroupEntry.toList ());
				};
				Id2ProfileField_ ["usejournals"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& val : entry.Value ())
						profile.Communities_ << val.toList ().value (0).toString ();
				};
				Id2ProfileField_ ["fullname"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.FullName_ = entry.ValueToString ();
				};
				Id2ProfileField_ ["moods"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& moodEntry : entry.Value ())
						profile.Moods_ << CreateMood (moodEntry.toList ());
				};
				Id2ProfileField_ ["userid"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.UserId_ = entry.ValueToLongLong ();
				};
				Id2ProfileField_ ["caps"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.Caps_ = entry.ValueToLongLong ();
				};
				Id2ProfileField_ ["pickws"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& val : entry.Value ())
						profile.AvatarsID_ << val.toList ().value (0).toString ();
				};
				Id2ProfileField_ ["pickwurls"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& val : entry.Value ())
						profile.AvatarsUrls_ << QUrl (val.toList ().value (0).toString ());
				};
			}
		};

		LJProfileData ParseProfileInfo (const QDomDocument& document)
		{
			static Id2ProfileField id2field;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			LJProfileData profile;
			if (firstStructElement.at (0).isNull ())
				return LJProfileData ();

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
						member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (id2field.Id2ProfileField_.contains (res.Name ()))
					id2field.Id2ProfileField_ [res.Name ()] (profile, res);
			}

			return profile;
		}
	}

	void LJXmlRPC::handleValidateReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit profileUpdated (ParseProfileInfo (document));
			CallNextFunctionFromQueue ();
			emit validatingFinished (true);
			return;
		}
		else
			emit validatingFinished (false);

		ParseForError (content);
	}

	void LJXmlRPC::handleRequestFriendsInfoFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			ParseFriends (document);
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleAddNewFriendReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			ParseFriends (document);
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleReplyWithProfileUpdate ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			Account_->updateProfile ();
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handlePreviewEventReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		const auto& path = QStandardPaths::writableLocation (QStandardPaths::TempLocation) +
				QString ("/blogique_preview_%1.bml")
						.arg (QDateTime::currentSecsSinceEpoch ());
		QFile file (path);
		if (file.open (QIODevice::WriteOnly))
		{
			file.write (reply->readAll ());
			file.close ();
			Proxy_->GetEntityManager ()->HandleEntity (Util::MakeEntity (QUrl::fromLocalFile (file.fileName ()),
					QString (), OnlyHandle | FromUserInitiated));
		}
	}

	namespace
	{
		int GetEventItemId (const QDomDocument& document)
		{
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return -1;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
					member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () == "itemid")
					return res.ValueToInt ();
			}

			return -1;
		}
	}

	void LJXmlRPC::handlePostEventReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			GetParticularEvent (GetEventItemId (document), RequestType::Post);
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleBackupEventsReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		Filter filter = Reply2Filter_.take (reply);
		if (document.elementsByTagName ("fault").isEmpty ())
		{
			auto eventsList = ParseFullEvents (Account_->GetOurLogin (), document);
			int count = eventsList.count ();
			if (count)
			{
				for (int i = count - 1; i >= 0; --i)
				{
					const auto& event = eventsList.at (i);
					if (filter.CustomDate_ && (event.DateTime_ < filter.BeginDate_))
						eventsList.removeAt (i);
					else if (!filter.Tags_.isEmpty ())
					{
						bool found = false;
						for (const auto& tag : filter.Tags_)
							if (event.Tags_.contains (tag))
							{
								found = true;
								break;
							}

						if (!found)
							eventsList.removeAt (i);
					}
				}

				emit gotFilteredEvents (eventsList);

				filter.Skip_ += count;
				GetEventsWithFilter (filter);
			}
			else
			{
				emit gettingFilteredEventsFinished ();
				CallNextFunctionFromQueue ();
			}
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleGotEventsReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit gotEvents (ParseFullEvents (Account_->GetOurLogin (), document));
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleGotEventsByDateReplyFinished ()
	{
		QDomDocument document;
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		const int skip = Reply2Skip_.take (reply);
		QDate dt = Reply2Date_.take (reply);

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			auto events = ParseFullEvents (Account_->GetOurLogin (), document);
			emit gotEvents (events);
			GetEventsByDate (dt, skip + events.count ());
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleRemoveEventReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit eventRemoved (GetEventItemId (document));
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleUpdateEventReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			GetParticularEvent (GetEventItemId (document), RequestType::Update);
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleGetParticularEventReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		RequestType rt = Reply2RequestType_.take (reply);
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			const auto& events = ParseFullEvents (Account_->GetOurLogin (), document);
			switch (rt)
			{
			case RequestType::Post:
				emit eventPosted (events);
				break;
			case RequestType::Update:
				emit eventUpdated (events);
				break;
			default:
				emit gotEvents (events);
				break;
			}
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleGetMultipleEventsReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			const auto& events = ParseFullEvents (Account_->GetOurLogin (), document);

			switch (Reply2RequestType_.take (reply))
			{
			case RequestType::RecentComments:
			{
				for (const auto& pairKey : Id2CommentEntry_.keys ())
					for (const auto& event : events)
					{
						if (event.ItemID_ != pairKey.first)
							continue;

						Id2CommentEntry_ [pairKey].NodeSubject_ = event.Event_;
						Id2CommentEntry_ [pairKey].NodeUrl_ = event.Url_;
						Id2CommentEntry_ [pairKey].ReplyId_ = 256 * Id2CommentEntry_ [pairKey].ReplyId_ + event.ANum_;
						Id2CommentEntry_ [pairKey].NodeId_ = 256 * Id2CommentEntry_ [pairKey].NodeId_ + event.ANum_;
					}

				auto comments = Id2CommentEntry_.values ();
				emit gotRecentComments (comments);
				Id2CommentEntry_.clear ();
				break;
			}
			default:
				break;
			}
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleBlogStatisticsReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit gotStatistics (ParseStatistics (document));
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	namespace
	{
		QList<int> GetUnreadMessagesIds (QDomDocument document)
		{
			QList<int> unreadIds;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return unreadIds;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
						member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () != "items")
					continue;

				for (const auto& message : res.Value ())
				{
					bool isUnread = false;
					int id = -1;
					for (const auto& field : message.toList ())
					{
						auto fieldEntry = field.value<LJParserTypes::LJParseProfileEntry> ();
						if (fieldEntry.Name () == "state")
							isUnread = fieldEntry.ValueToString ().toLower () == "n";
						if (fieldEntry.Name () == "qid")
							id = fieldEntry.ValueToInt ();
					}

					if (isUnread && id != -1)
						unreadIds << id;
				}
			}
			return unreadIds;
		}
	}

	void LJXmlRPC::handleInboxReplyFinished ()
	{
		QDomDocument document;
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			const auto& unreadIds = GetUnreadMessagesIds (document);
			if (!unreadIds.isEmpty ())
				emit unreadMessagesIds (unreadIds);
			XmlSettingsManager::Instance ().setProperty ("LastInboxUpdateDate",
					QDateTime::currentDateTime ());
			CallNextFunctionFromQueue ();
			return;
		}
		ParseForError (content);
	}

	void LJXmlRPC::handleMessagesSetAsReadFinished ()
	{
		QDomDocument document;
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit messagesRead ();
			CallNextFunctionFromQueue ();
			return;
		}
		ParseForError (content);
	}

	void LJXmlRPC::handleSendMessageRequestFinished ()
	{
		QDomDocument document;
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		QByteArray content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit messageSent ();
			CallNextFunctionFromQueue ();
			return;
		}
		ParseForError (content);
	}

	namespace
	{
		LJCommentEntry ParseComment (const QVariantList& comments)
		{
			LJCommentEntry comment;
			for (const auto& field : comments)
			{
				auto fieldEntry = field.value<LJParserTypes::LJParseProfileEntry> ();
				if (fieldEntry.Name () == "nodeid")
					comment.NodeId_ = fieldEntry.ValueToInt ();
				else if (fieldEntry.Name () == "subject")
					comment.Subject_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "posterid")
					comment.PosterId_ = fieldEntry.ValueToInt ();
				else if (fieldEntry.Name () == "state")
				{
					CommentState state = CommentState::Active;
					if (fieldEntry.ValueToString ().toLower () == "f")
						state = CommentState::Frozen;
					else if (fieldEntry.ValueToString ().toLower () == "s")
						state = CommentState::Secure;
					else if (fieldEntry.ValueToString ().toLower () == "d")
						state = CommentState::Deleted;
					else
						state = CommentState::Active;
					comment.State_ = state;
				}
				else if (fieldEntry.Name () == "jtalkid")
					comment.ReplyId_ = fieldEntry.ValueToInt ();
				else if (fieldEntry.Name () == "parenttalkid")
					comment.ParentReplyId_ = fieldEntry.ValueToInt ();
				else if (fieldEntry.Name () == "postername")
					comment.PosterName_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "text")
					comment.Text_ = fieldEntry.ValueToString ();
				else if (fieldEntry.Name () == "datepostunix")
					comment.PostingDate_ = QDateTime::fromSecsSinceEpoch (fieldEntry.ValueToLongLong ());
			}

			return comment;
		}

		QList<LJCommentEntry> ParseComments (const QDomDocument& document)
		{
			QList<LJCommentEntry> comments;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return comments;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
						member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () == "comments")
					for (const auto& message : res.Value ())
						comments << ParseComment (message.toList ());
			}

			return comments;
		}
	}

	void LJXmlRPC::handleRecentCommentsReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			QList<int> ids;
			for (auto comment : ParseComments (document))
			{
				auto pairKey = qMakePair (comment.NodeId_, comment.ReplyId_);
				if (!Id2CommentEntry_.contains (pairKey))
				{
					Id2CommentEntry_ [pairKey] = comment;
					ids << comment.NodeId_;
				}
			}

			ids.removeAll (0);
			if (!ids.isEmpty ())
				GetMultiplyEvents (ids, RequestType::RecentComments);
			else
				CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	namespace
	{
		QList<qint64> ParseCommentIds (const QDomDocument& document)
		{
			QList<qint64> ids;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return ids;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
						member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () == "dtalkids")
					for (const auto& dtalkid : res.Value ())
						for (const auto& id : dtalkid.toList ())
							ids << id.toLongLong ();
			}
			return ids;
		}
	}

	void LJXmlRPC::handleDeleteCommentReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit commentsDeleted (ParseCommentIds (document));
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	namespace
	{
		QUrl ParseNewCommentUrl (const QDomDocument& document)
		{
			QUrl url;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return url;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
						member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () == "commentlink")
					url = res.ValueToUrl ();
			}

			return url;
		}
	}

	void LJXmlRPC::handleAddCommentReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			auto url = ParseNewCommentUrl (document);
			if (url.isValid ())
				emit commentSent (url);

			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	namespace
	{
		QHash<QString, int> ParseTags (const QDomDocument& document)
		{
			QHash<QString, int> tags;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (firstStructElement.at (0).isNull ())
				return tags;

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
					member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (res.Name () != "tags")
					continue;
				for (const auto& tag : res.Value ())
				{
					QString name;
					int uses = 0;
					for (const auto& tagStruct : tag.toList ())
					{
						auto fieldEntry = tagStruct.value<LJParserTypes::LJParseProfileEntry> ();
						if (fieldEntry.Name () == "name")
							name = fieldEntry.ValueToString ();
						else if (fieldEntry.Name () == "uses")
							uses = fieldEntry.ValueToInt ();
					}
					tags [name] = uses;
				}
			}
			return tags;
		}
	}

	void LJXmlRPC::handleGetUserTagsReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit gotTags (ParseTags (document));
			CallNextFunctionFromQueue ();
			return;
		}

		ParseForError (content);
	}

	void LJXmlRPC::handleNetworkError (QNetworkReply::NetworkError err)
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();
		qWarning () << Q_FUNC_INFO << err << reply->errorString ();
		emit networkError (err, reply->errorString ());
		CallNextFunctionFromQueue ();
	}

}
}
}
