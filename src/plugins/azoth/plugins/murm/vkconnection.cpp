/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkconnection.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QtDebug>
#include <util/svcauth/vkauthmanager.h>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include <util/sll/prelude.h>
#include <util/sll/slotclosure.h>
#include "longpollmanager.h"
#include "logger.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	namespace
	{
		const QString UserFields { "first_name,last_name,nickname,photo,photo_big,sex,"
				"bdate,city,country,timezone,contacts,education,online,online_mobile" };

		QStringList GetPerms ()
		{
			QStringList result { "messages", "notifications", "friends", "status", "photos", "audio", "docs" };
			if (XmlSettingsManager::Instance ().property ("RequireOffline").toBool ())
				result << "offline";
			return result;
		}

		const QString CurrentAPIVersion { "5.25" };

		QNetworkReply* Autodelete (QNetworkReply *reply)
		{
			QObject::connect (reply,
					&QNetworkReply::finished,
					reply,
					&QObject::deleteLater);
			return reply;
		}
	}

	VkConnection::CommandException::CommandException (const QString& str)
	: runtime_error { str.toUtf8 ().constData () }
	{
	}

	VkConnection::RecoverableException::RecoverableException ()
	: CommandException { "VK connection recoverable error" }
	{
	}

	VkConnection::UnrecoverableException::UnrecoverableException (int code, const QString& msg)
	: CommandException { "VK connection error code " + QString::number (code) + "; " + msg }
	, Code_ { code }
	, Msg_ { msg }
	{
	}

	int VkConnection::UnrecoverableException::GetCode () const
	{
		return Code_;
	}

	const QString& VkConnection::UnrecoverableException::GetMessage () const
	{
		return Msg_;
	}

	VkConnection::VkConnection (const QString& name,
			const QByteArray& cookies, ICoreProxy_ptr proxy, Logger& logger)
	: AuthMgr_ (new Util::SvcAuth::VkAuthManager (name, "3778319",
			GetPerms (),
			cookies, proxy, nullptr, this))
	, Proxy_ (proxy)
	, Logger_ (logger)
	, LastCookies_ (cookies)
	, CallQueue_ (new Util::QueueManager (400))
	, LPManager_ (new LongPollManager (this, proxy))
	, MarkOnlineTimer_ (new QTimer (this))
	{
		Logger_ << "==========================================";
		connect (AuthMgr_,
				SIGNAL (cookiesChanged (QByteArray)),
				this,
				SLOT (saveCookies (QByteArray)));
		connect (AuthMgr_,
				SIGNAL (gotAuthKey (QString)),
				this,
				SLOT (callWithKey (QString)));

		connect (LPManager_,
				SIGNAL (listening ()),
				this,
				SLOT (handleListening ()));
		connect (LPManager_,
				SIGNAL (stopped ()),
				this,
				SLOT (handlePollStopped ()));
		connect (LPManager_,
				SIGNAL (pollError ()),
				this,
				SLOT (handlePollError ()));
		connect (LPManager_,
				SIGNAL (gotPollData (QVariantMap)),
				this,
				SLOT (handlePollData (QVariantMap)));

		Dispatcher_ [1] = [] (const QVariantList&) {};
		Dispatcher_ [2] = [] (const QVariantList&) {};
		Dispatcher_ [3] = [] (const QVariantList&) {};

		Dispatcher_ [4] = [this] (const QVariantList& items) { HandleMessage (items); };
		Dispatcher_ [8] = [this] (const QVariantList& items)
			{ emit userStateChanged (items.value (1).toLongLong () * -1, true); };
		Dispatcher_ [9] = [this] (const QVariantList& items)
			{ emit userStateChanged (items.value (1).toLongLong () * -1, false); };
		Dispatcher_ [51] = [this] (const QVariantList& items)
			{ emit mucChanged (items.value (1).toLongLong ()); };
		Dispatcher_ [61] = [this] (const QVariantList& items)
			{ emit gotTypingNotification (items.value (1).toULongLong ()); };

		// Stuff has been read, we don't care
		Dispatcher_ [6] = [] (const QVariantList&) {};
		Dispatcher_ [7] = [] (const QVariantList&) {};

		// Unread counter value change, we dont' care
		Dispatcher_ [80] = [] (const QVariantList&) {};

		Dispatcher_ [101] = [] (const QVariantList&) {};	// unknown stuff

		MarkOnlineTimer_->setInterval (12 * 60 * 1000);
		connect (MarkOnlineTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (markOnline ()));

		XmlSettingsManager::Instance ().RegisterObject ("RequireOffline",
				this, "handleScopeSettingsChanged");
	}

	const QByteArray& VkConnection::GetCookies () const
	{
		return LastCookies_;
	}

	void VkConnection::RerequestFriends ()
	{
		Logger_ (IHaveConsole::PacketDirection::Out) << "RerequestFriends";
		PushFriendsRequest ();
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::SendMessage (qulonglong to, const QString& body,
			std::function<void (qulonglong)> idSetter, Type type, const QByteArrayList& attachments)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.send");

				auto query = "access_token=" + QUrl::toPercentEncoding (key.toUtf8 ());
				query += '&';
				query += type == Type::Dialog ? "user_id" : "chat_id";
				query += '=' + QByteArray::number (to);
				query += "&type=1";
				query += "&message=" + QUrl::toPercentEncoding (body, {}, "+");

				if (!attachments.isEmpty ())
					query += "&attachment=" + attachments.join (",");

				for (auto i = params.begin (); i != params.end (); ++i)
					query += "&" + QUrl::toPercentEncoding (i.key ()) +
							"=" + QUrl::toPercentEncoding (i.value ());

				QNetworkRequest req (url);
				req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
				auto reply = nam->post (req, query);
				Logger_ (IHaveConsole::PacketDirection::Out)
						<< url
						<< " : posting"
						<< QString::fromUtf8 (query);
				MsgReply2Setter_ [reply] = idSetter;
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleMessageSent ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::SendTyping (qulonglong to)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([nam, to] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.setActivity");
				Util::UrlOperator { url }
						("access_token", key)
						("user_id", QString::number (to))
						("type", "typing");

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	namespace
	{
		template<typename T>
		QString CommaJoin (const QList<T>& ids)
		{
			QStringList converted;
			for (auto id : ids)
				converted << QString::number (id);
			return converted.join (",");
		}
	}

	void VkConnection::MarkAsRead (const QList<qulonglong>& ids)
	{
		const auto& joined = CommaJoin (ids);

		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([nam, joined] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.markAsRead");
				Util::UrlOperator { url }
						("access_token", key)
						("message_ids", joined);

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::RequestGeoIds (const QList<int>& codes, GeoSetter_f setter, GeoIdType type)
	{
		if (codes.isEmpty ())
			return;

		const auto& joined = CommaJoin (codes);

		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QString method;
				QString paramName;
				switch (type)
				{
				case GeoIdType::Country:
					method = "Countries";
					paramName = "country_ids";
					break;
				case GeoIdType::City:
					method = "Cities";
					paramName = "city_ids";
					break;
				}

				QUrl url ("https://api.vk.com/method/database.get" + method + "ById");
				Util::UrlOperator { url }
						("access_token", key)
						(paramName, joined);

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				CountryReply2Setter_ [reply] = setter;
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleCountriesFetched ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::GetUserInfo (const QList<qulonglong>& ids)
	{
		GetUserInfo (ids,
				[this] (const QList<UserInfo>& ids) { emit gotUsers (ids); });
	}

	namespace
	{
		AppInfo UserMap2AppInfo (const QVariantMap& userMap)
		{
			return
			{
				userMap ["online_app"].toULongLong (),
				userMap ["online_mobile"].toBool (),
				{},
				{}
			};
		}

		AppInfo AppMap2AppInfo (const QVariantMap& appMap)
		{
			return
			{
				appMap ["id"].toULongLong (),
				true,
				appMap ["title"].toString (),
				QUrl::fromEncoded (appMap ["icon_25"].toByteArray ())
			};
		}

		UserInfo UserMap2Info (const QVariantMap& userMap)
		{
			QList<qulonglong> lists;
			for (const auto& item : userMap ["lists"].toList ())
				lists << item.toULongLong ();

			auto dateString = userMap ["bdate"].toString ();
			if (dateString.count ('.') == 1)
				dateString += ".1800";

			const auto& contacts = userMap ["contacts"].toMap ();

			const auto& countryMap = userMap ["country"].toMap ();
			const auto& cityMap = userMap ["city"].toMap ();

			return
			{
				userMap ["id"].toULongLong (),

				userMap ["first_name"].toString (),
				userMap ["last_name"].toString (),
				userMap ["nickname"].toString (),

				QUrl (userMap ["photo"].toString ()),
				QUrl (userMap ["photo_big"].toString ()),

				userMap ["sex"].toInt (),

				QDate::fromString (dateString, "d.M.yyyy"),

				contacts ["home_phone"].toString (),
				contacts ["mobile_phone"].toString (),

				userMap ["timezone"].toInt (),

				countryMap ["id"].toInt (),
				countryMap ["title"].toString (),
				cityMap ["id"].toInt (),
				cityMap ["title"].toString (),

				static_cast<bool> (userMap ["online"].toULongLong ()),

				lists,

				UserMap2AppInfo (userMap)
			};
		}

		QList<UserInfo> ParseUsers (const QVariantList& usersList)
		{
			QList<UserInfo> users;

			for (const auto& item : usersList)
			{
				const auto& userMap = item.toMap ();
				if (userMap.contains ("deactivated"))
					continue;

				users << UserMap2Info (userMap);
			}

			return users;
		}
	}

	void VkConnection::GetUserInfo (const QList<qulonglong>& ids,
			const std::function<void (QList<UserInfo>)>& cont)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		const auto& joined = CommaJoin (ids);
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/users.get");
				Util::UrlOperator { url }
						("access_token", key)
						("fields", UserFields);
				if (!joined.isEmpty ())
					Util::UrlOperator { url } ("user_ids", joined);

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest { url });
				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[this, reply, cont]
					{
						if (!CheckFinishedReply (reply))
							return;

						const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
						Logger_ << "got users reply" << data;
						try
						{
							CheckReplyData (data, reply);
						}
						catch (const CommandException&)
						{
							return;
						}

						cont (ParseUsers (data.toMap () ["response"].toList ()));
					},
					reply,
					SIGNAL (finished ()),
					reply
				};
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::RequestUserAppId (qulonglong id)
	{
		const auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/users.get");
				Util::UrlOperator { url }
						("access_token", key)
						("user_ids", QString::number (id))
						("fields", "online,online_mobile");

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest { url });
				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[this, reply, id]
					{
						if (!CheckFinishedReply (reply))
							return;

						const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
						Logger_ << "got users app data" << data;
						try
						{
							CheckReplyData (data, reply);
						}
						catch (const CommandException&)
						{
							return;
						}

						const auto& users = data.toMap () ["response"].toList ();
						const auto appId = users.value (0).toMap () ["online_app"].toULongLong ();
						const auto isMobile = users.value (0).toMap () ["online_mobile"].toBool ();
						emit gotUserAppInfoStub (id, { appId, isMobile, {}, {} });
					},
					reply,
					SIGNAL (finished ()),
					reply
				};
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::GetMessageInfo (qulonglong id, MessageInfoSetter_f setter)
	{
		GetMessageInfo (QString::number (id), setter);
	}

	void VkConnection::GetMessageInfo (const QString& idStr, MessageInfoSetter_f setter)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.getById");
				Util::UrlOperator { url }
						("access_token", key)
						("message_ids", idStr)
						("photo_sizes", "1");

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				Reply2MessageSetter_ [reply] = setter;
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleMessageInfoFetched ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::GetAppInfo (qulonglong appId, const std::function<void (AppInfo)>& setter)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString&, const UrlParams_t& params)
			{
				QUrl url { "https://api.vk.com/method/apps.get" };
				Util::UrlOperator { url }
					("app_id", QString::number (appId));

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest { url });
				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[reply, setter]
					{
						reply->deleteLater ();

						const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();
						if (data.isEmpty ())
							return;

						setter (AppMap2AppInfo (data ["response"].toMap ()));
					},
					reply,
					SIGNAL (finished ()),
					reply
				};
				return reply;
			});
	}

	void VkConnection::AddFriendList (const QString& name, const QList<qulonglong>& ids)
	{
		const auto& joined = CommaJoin (ids);
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([this, joined, name, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/friends.addList");
				Util::UrlOperator { url }
						("access_token", key)
						("name", name)
						("user_ids", joined);

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				Reply2ListName_ [reply] = name;
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleFriendListAdded ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::ModifyFriendList (const ListInfo& list, const QList<qulonglong>& newContents)
	{
		const auto& joined = CommaJoin (newContents);
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([joined, list, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/friends.editList");
				Util::UrlOperator { url }
						("access_token", key)
						("list_id", QString::number (list.ID_))
						("name", list.Name_)
						("user_ids", joined);

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::SetNRIList (const QList<qulonglong>& ids)
	{
		const auto& joined = CommaJoin (ids);
		auto nam = Proxy_->GetNetworkAccessManager ();

		PreparedCalls_.push_back ([joined, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/storage.set");
				Util::UrlOperator { url }
						("access_token", key)
						("key", "non_roster_items")
						("value", joined);

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::CreateChat (const QString& title, const QList<qulonglong>& ids)
	{
		const auto& joined = CommaJoin (ids);
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.createChat");
				Util::UrlOperator { url }
						("access_token", key)
						("title", title)
						("user_ids", joined);

				AddParams (url, params);

				const auto& infos = Util::Map (ids, &UserInfo::FromID);
				auto reply = nam->get (QNetworkRequest (url));
				Reply2ChatInfo_ [reply] = { 0, title, infos };
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleChatCreated ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::RequestChatInfo (qulonglong id)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.getChat");
				Util::UrlOperator { url }
						("access_token", key)
						("chat_id", QString::number (id))
						("fields", UserFields);

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleChatInfo ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::AddChatUser (qulonglong chat, qulonglong user)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.addChatUser");
				Util::UrlOperator { url }
						("access_token", key)
						("chat_id", QString::number (chat))
						("user_id", QString::number (user));

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::RemoveChatUser (qulonglong chat, qulonglong user)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.removeChatUser");
				Util::UrlOperator { url }
						("access_token", key)
						("chat_id", QString::number (chat))
						("user_id", QString::number (user));

				AddParams (url, params);

				auto reply = nam->get (QNetworkRequest (url));
				Reply2ChatRemoveInfo_ [reply] = ChatRemoveInfo { chat, user };
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleChatUserRemoved ()));
				return reply;
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::SetChatTitle (qulonglong chat, const QString& title)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/messages.editChat");
				Util::UrlOperator { url }
						("access_token", key)
						("chat_id", QString::number (chat))
						("title", title);

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::SetStatus (QString status)
	{
		if (status.isEmpty ())
			status = Status_.StatusString_;

		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([=] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/status.set");
				Util::UrlOperator { url }
						("access_token", key)
						("text", status);

				AddParams (url, params);

				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::SetStatus (const EntryStatus& status, bool updateString)
	{
		Logger_ (IHaveConsole::PacketDirection::Out) << "setting status" << status.State_;
		LPManager_->ForceServerRequery ();

		Status_ = status;
		if (Status_.State_ == SOffline)
		{
			LPManager_->Stop ();

			PreparedCalls_.clear ();
			RunningCalls_.clear ();
			CallQueue_->Clear ();

			return;
		}

		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([this, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl lpUrl ("https://api.vk.com/method/users.get");
				Util::UrlOperator { lpUrl }
						("access_token", key)
						("fields",
								"first_name,last_name,nickname,photo,photo_big,sex,"
								"bdate,city,country,timezone,contacts,education");
				AddParams (lpUrl, params);
				auto reply = nam->get (QNetworkRequest (lpUrl));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotSelfInfo ()));
				return reply;
			});
		PreparedCalls_.push_back ([this, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl lpUrl ("https://api.vk.com/method/friends.getLists");
				Util::UrlOperator { lpUrl } ("access_token", key);
				AddParams (lpUrl, params);
				auto reply = nam->get (QNetworkRequest (lpUrl));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotFriendLists ()));
				return reply;
			});

		if (updateString)
			SetStatus (Status_.StatusString_);

		AuthMgr_->GetAuthKey ();
	}

	EntryStatus VkConnection::GetStatus () const
	{
		return CurrentStatus_;
	}

	void VkConnection::SetMarkingOnlineEnabled (bool enabled)
	{
		Logger_ (IHaveConsole::PacketDirection::Out) << "SetMarkingOnlineEnabled" << enabled;
		MarkingOnline_ = enabled;

		if (enabled)
		{
			markOnline ();
			MarkOnlineTimer_->start ();
		}
		else
			MarkOnlineTimer_->stop ();
	}

	void VkConnection::QueueRequest (VkConnection::PreparedCall_f call)
	{
		PreparedCalls_ << call;
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::AddParams (QUrl& url, const UrlParams_t& params)
	{
		Util::UrlOperator op { url };
		for (auto i = params.begin (); i != params.end (); ++i)
			op (i.key (), i.value ());
	}

	void VkConnection::HandleCaptcha (const QString& cid, const QString& value)
	{
		if (!CaptchaId2Call_.contains (cid))
			return;

		auto call = CaptchaId2Call_.take (cid);
		if (value.isEmpty ())
			return;

		call.ClearParams ();
		call.AddParam ({ "captcha_sid", cid });
		call.AddParam ({ "captcha_img", value });

		PreparedCalls_.push_front (call);

		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::HandleMessage (const QVariantList& items)
	{
		const auto& params = items.value (7).toMap ();
		MessageInfo info
		{
			items.value (1).toULongLong (),
			items.value (3).toULongLong (),
			items.value (6).toString (),
			MessageFlags { items.value (2).toInt () },
			QDateTime::fromSecsSinceEpoch (items.value (4).toULongLong ()),
			params
		};

		if (info.Params_.contains ("from"))
		{
			info.From_ -= 2000000000;
			info.Flags_ |= MessageFlag::Chat;
		}
		else
			info.Flags_ &= ~MessageFlag::Chat;

		if (info.Params_.contains ("fwd"))
		{
			GetMessageInfo (info.ID_,
					[this, info] (const FullMessageInfo& fullInfo)
					{
						emit gotMessage (fullInfo, info);
					});
			return;
		}

		emit gotMessage (info);
	}

	void VkConnection::PushFriendsRequest ()
	{
		auto nam = Proxy_->GetNetworkAccessManager ();

		PreparedCalls_.push_back ([this, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl friendsUrl ("https://api.vk.com/method/friends.get");
				Util::UrlOperator { friendsUrl }
						("access_token", key)
						("fields", UserFields);
				AddParams (friendsUrl, params);
				auto reply = nam->get (QNetworkRequest (friendsUrl));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotFriends ()));
				return reply;
			});

		PreparedCalls_.push_back ([this, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/storage.get");
				Util::UrlOperator { url }
						("access_token", key)
						("key", "non_roster_items");
				AddParams (url, params);
				auto reply = nam->get (QNetworkRequest (url));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotNRI ()));
				return reply;
			});
	}

	auto VkConnection::FindRunning (QNetworkReply *reply) const -> RunningCalls_t::const_iterator
	{
		return std::find_if (RunningCalls_.begin (), RunningCalls_.end (),
				[reply] (const auto& call) { return call.first == reply; });
	}

	auto VkConnection::FindRunning (QNetworkReply *reply) -> RunningCalls_t::iterator
	{
		return std::find_if (RunningCalls_.begin (), RunningCalls_.end (),
				[reply] (const auto& call) { return call.first == reply; });
	}

	void VkConnection::RescheduleRequest (QNetworkReply *reply)
	{
		const auto pos = FindRunning (reply);
		if (pos != RunningCalls_.end ())
			PreparedCalls_.push_front (pos->second);
		else
			qWarning () << Q_FUNC_INFO
					<< "no running call found for the reply";
	}

	bool VkConnection::CheckFinishedReply (QNetworkReply *reply)
	{
		reply->deleteLater ();

		if (reply->error () == QNetworkReply::NoError)
		{
			APIErrorCount_ = 0;
			return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "reply error:"
				<< reply->error ()
				<< reply->errorString ();

		RescheduleRequest (reply);

		++APIErrorCount_;

		if (!ShouldRerunPrepared_)
		{
			QTimer::singleShot (30000,
					this,
					SLOT (rerunPrepared ()));
			ShouldRerunPrepared_ = true;
		}

		return false;
	}

	void VkConnection::CheckReplyData (const QVariant& mapVar, QNetworkReply *reply)
	{
		const auto& map = mapVar.toMap ();
		if (!map.contains ("error"))
			return;

		const auto& errMap = map ["error"].toMap ();
		const auto ec = errMap ["error_code"].toInt ();
		const auto& errMsg = errMap ["error_msg"].toString ();

		Logger_ << "got error:" << ec << errMsg;
		Logger_ << errMap;

		switch (ec)
		{
		case 5:
			RescheduleRequest (reply);
			reauth ();
			throw RecoverableException {};
		case 14:
		{
			const auto pos = FindRunning (reply);
			if (pos == RunningCalls_.end ())
			{
				qWarning () << Q_FUNC_INFO
						<< "non-running reply captcha";
				break;
			}

			const auto& cid = errMap ["captcha_sid"].toString ();
			const auto& img = errMap ["captcha_img"].toString ();

			CaptchaId2Call_ [cid] = pos->second;

			emit captchaNeeded (cid, QUrl::fromEncoded (img.toUtf8 ()));

			throw RecoverableException {};
		}
		}

		throw UnrecoverableException { ec, errMsg };
	}

	void VkConnection::reauth ()
	{
		Logger_ << "reauthing";
		AuthMgr_->ClearAuthData ();
		LPManager_->ForceServerRequery ();
		LPManager_->start ();
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::rerunPrepared ()
	{
		ShouldRerunPrepared_ = false;

		if (!PreparedCalls_.isEmpty ())
			AuthMgr_->GetAuthKey ();
	}

	void VkConnection::callWithKey (const QString& key)
	{
		while (!PreparedCalls_.isEmpty ())
		{
			auto f = PreparedCalls_.takeFirst ();
			f.AddParam ({ "v", CurrentAPIVersion });
			CallQueue_->Schedule ([this, f, key]
					{
						const auto reply = f (key);
						if (!reply)
						{
							qWarning () << Q_FUNC_INFO
									<< "the prepared call returned a null reply";
							return;
						}

						Logger_ (IHaveConsole::PacketDirection::Out) << reply->request ().url ();
						RunningCalls_.append ({ reply, f });

						connect (reply,
								SIGNAL (destroyed ()),
								this,
								SLOT (handleReplyDestroyed ()));
					});
		}
	}

	void VkConnection::handleReplyDestroyed ()
	{
		const auto reply = static_cast<QNetworkReply*> (sender ());
		const auto pos = FindRunning (reply);

		if (pos == RunningCalls_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "finished a non-running reply";
			return;
		}

		RunningCalls_.erase (pos);
	}

	void VkConnection::markOnline ()
	{
		if (Status_.State_ != SOnline &&
				Status_.State_ != SChat)
			return;

		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl url ("https://api.vk.com/method/account.setOnline");
				Util::UrlOperator { url } ("access_token", key);
				AddParams (url, params);
				return Autodelete (nam->get (QNetworkRequest (url)));
			});
		Logger_ (IHaveConsole::PacketDirection::Out) << "markOnline";
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::handleListening ()
	{
		Logger_ << "listening now";
		CurrentStatus_ = Status_;
		emit statusChanged (GetStatus ());

		SetMarkingOnlineEnabled (MarkingOnline_);
	}

	void VkConnection::handlePollError ()
	{
		Logger_ << "poll error";
		CurrentStatus_ = EntryStatus ();
		emit statusChanged (GetStatus ());
	}

	void VkConnection::handlePollStopped ()
	{
		Logger_ << "poll stopped";
		CurrentStatus_ = Status_;
		emit statusChanged (GetStatus ());

		emit stoppedPolling ();

		MarkOnlineTimer_->stop ();
	}

	void VkConnection::handlePollData (const QVariantMap& rootMap)
	{
		Logger_ << "got poll data" << rootMap;
		for (const auto& update : rootMap ["updates"].toList ())
		{
			const auto& parmList = update.toList ();
			const auto code = parmList.value (0).toInt ();

			if (!Dispatcher_.contains (code))
				qWarning () << Q_FUNC_INFO
						<< "unknown code"
						<< code
						<< parmList;
			else
				Dispatcher_ [code] (parmList);
		}
	}

	void VkConnection::handleFriendListAdded ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& name = Reply2ListName_.take (reply);

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		bool converted = false;
		const auto id = data.toMap () ["response"].toMap () ["lid"].toULongLong (&converted);
		if (!converted)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply"
					<< data;
			return;
		}

		emit addedLists ({ { id, name } });
	}

	void VkConnection::handleGotSelfInfo ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		const auto& list = data.toMap () ["response"].toList ();
		const auto& selfMap = list.value (0).toMap ();
		if (selfMap.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "null self map";
			return;
		}
		emit gotSelfInfo (UserMap2Info (selfMap));
	}

	void VkConnection::handleGotFriendLists ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		QList<ListInfo> lists;
		const auto& responseList = data.toMap () ["response"].toMap () ["items"].toList ();
		for (const auto& item : responseList)
		{
			const auto& map = item.toMap ();
			lists.append ({ map ["id"].toULongLong (), map ["name"].toString () });
		}
		emit gotLists (lists);

		PushFriendsRequest ();
		AuthMgr_->GetAuthKey ();
	}

	void VkConnection::handleGotFriends ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		const auto& users = ParseUsers (data.toMap () ["response"].toMap () ["items"].toList ());
		emit gotUsers (users);

		auto nam = Proxy_->GetNetworkAccessManager ();
		PreparedCalls_.push_back ([this, nam] (const QString& key, const UrlParams_t& params)
			{
				QUrl msgUrl ("https://api.vk.com/method/messages.get");
				Util::UrlOperator { msgUrl }
						("access_token", key)
						("photo_sizes", "1");
				AddParams (msgUrl, params);
				auto reply = nam->get (QNetworkRequest (msgUrl));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotUnreadMessages ()));
				return reply;
			});

		LPManager_->start ();
	}

	void VkConnection::handleGotNRI ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		const auto& str = data.toMap () ["response"].toString ();
		QList<qulonglong> ids;
		for (const auto& sub : str.splitRef (",", Qt::SkipEmptyParts))
		{
			bool ok = false;
			const auto id = sub.toULongLong (&ok);
			if (ok)
				ids << id;
		}

		emit gotNRIList (ids);
	}

	namespace
	{
		PhotoInfo PhotoMap2Info (const QVariantMap& map)
		{
			QString bigSrc;
			QSize bigSize;
			QString thumbSrc;
			QSize thumbSize;

			QString currentBigType;
			const QStringList bigTypes { "x", "y", "z", "w" };

			for (const auto& elem : map ["sizes"].toList ())
			{
				const auto& eMap = elem.toMap ();

				auto size = [&eMap]
				{
					return QSize (eMap ["width"].toInt (), eMap ["height"].toInt ());
				};

				const auto& type = eMap ["type"].toString ();
				if (type == "m")
				{
					thumbSrc = eMap ["src"].toString ();
					thumbSize = size ();
				}
				else if (bigTypes.indexOf (type) > bigTypes.indexOf (currentBigType))
				{
					currentBigType = type;
					bigSrc = eMap ["src"].toString ();
					bigSize = size ();
				}
			}

			return
			{
				map ["owner_id"].toLongLong (),
				map ["id"].toULongLong (),
				map ["album_id"].toLongLong (),

				thumbSrc,
				thumbSize,
				bigSrc,
				bigSize,

				map ["access_key"].toString ()
			};
		}

		AudioInfo AudioMap2Info (const QVariantMap& map)
		{
			return
			{
				map ["owner_id"].toLongLong (),
				map ["id"].toULongLong (),
				map ["artist"].toString (),
				map ["title"].toString (),
				map ["duration"].toInt (),
				map ["url"].toString ()
			};
		}

		void HandleBasicMsgInfo (FullMessageInfo& info, const QVariantMap& wallMap)
		{
			info.OwnerID_ = wallMap.contains ("owner_id") ?
					wallMap ["owner_id"].toLongLong () :
					wallMap ["to_id"].toLongLong ();
			info.ID_ = wallMap ["id"].toULongLong ();
			info.Text_ = wallMap.contains ("text") ?
					wallMap ["text"].toString () :
					wallMap ["body"].toString ();
			info.Likes_ = wallMap ["likes"].toMap () ["count"].toInt ();
			info.Reposts_ = wallMap ["reposts"].toMap () ["count"].toInt ();
			info.PostDate_ = QDateTime::fromSecsSinceEpoch (wallMap ["date"].toLongLong ());
		}

		VideoInfo VideoMap2Info (const QVariantMap& map)
		{
			return
			{
				map ["owner_id"].toLongLong (),
				map ["id"].toULongLong (),
				map ["access_key"].toString (),
				map ["title"].toString (),
				map ["description"].toString (),
				map ["duration"].toULongLong (),
				map ["views"].toLongLong (),
				map ["photo_320"].toString ()
			};
		}

		DocumentInfo DocMap2Info (const QVariantMap& map)
		{
			return
			{
				map ["owner_id"].toLongLong (),
				map ["id"].toULongLong (),
				map ["title"].toString (),
				map ["ext"].toString (),
				map ["size"].toULongLong (),
				map ["url"].toString ()
			};
		}

		GiftInfo GiftMap2Info (const QVariantMap& map)
		{
			return
			{
				map ["id"].toULongLong (),
				QUrl::fromEncoded (map ["thumb_256"].toByteArray ())
			};
		}

		StickerInfo StickerMap2Info (const QVariantMap& map)
		{
			return
			{
				map ["id"].toString ()
			};
		}

		PagePreview PagePreviewMap2Info (const QVariantMap& map)
		{
			return
			{
				map ["gid"].toLongLong (),
				map ["pid"].toULongLong (),
				map ["title"].toString (),
				map ["view_url"].toString ()
			};
		}

		void HandleAttachments (FullMessageInfo& info, const QVariant& attachments, Logger& logger)
		{
			const auto& attList = attachments.toList ();
			for (const auto& attVar : attList)
			{
				const auto& attMap = attVar.toMap ();
				if (attMap.contains ("photo"))
					info.Photos_ << PhotoMap2Info (attMap ["photo"].toMap ());
				else if (attMap.contains ("audio"))
					info.Audios_ << AudioMap2Info (attMap ["audio"].toMap ());
				else if (attMap.contains ("video"))
					info.Videos_ << VideoMap2Info (attMap ["video"].toMap ());
				else if (attMap.contains ("doc"))
					info.Documents_ << DocMap2Info (attMap ["doc"].toMap ());
				else if (attMap.contains ("gift"))
					info.Gifts_ << GiftMap2Info (attMap ["gift"].toMap ());
				else if (attMap.contains ("sticker"))
					info.Stickers_ << StickerMap2Info (attMap ["sticker"].toMap ());
				else if (attMap.contains ("page"))
					info.PagesPreviews_ << PagePreviewMap2Info (attMap ["page"].toMap ());
				else if (attMap.contains ("wall"))
				{
					auto wallMap = attMap ["wall"].toMap ();

					FullMessageInfo repost;
					HandleBasicMsgInfo (repost, wallMap);

					if (wallMap.contains ("attachments"))
					{
						HandleAttachments (repost, wallMap.take ("attachments"), logger);
						logger << "attachments left:" << wallMap;
					}
					if (wallMap.contains ("copy_history"))
					{
						auto history = wallMap.take ("copy_history");
						for (const auto& obj : history.toList ())
						{
							FullMessageInfo copyItem;
							HandleBasicMsgInfo (copyItem, obj.toMap ());
							HandleAttachments (copyItem, obj.toMap () ["attachments"], logger);
							repost.ContainedReposts_ << copyItem;
						}
					}

					info.ContainedReposts_ << repost;
				}
				else
					logger << "HandleAttachments" << attMap.keys ();
			}
		}

		FullMessageInfo GetFullMessageInfo (const QVariantMap& map, Logger& logger);

		void HandleForwarded (FullMessageInfo& info, const QVariant& fwds, Logger& logger)
		{
			const auto& fwdList = fwds.toList ();
			for (const auto& fwdVar : fwdList)
				info.ForwardedMessages_ << GetFullMessageInfo (fwdVar.toMap (), logger);
		}

		FullMessageInfo GetFullMessageInfo (const QVariantMap& map, Logger& logger)
		{
			FullMessageInfo info;
			HandleBasicMsgInfo (info, map);
			HandleAttachments (info, map ["attachments"], logger);
			HandleForwarded (info, map ["fwd_messages"], logger);
			return info;
		}
	}

	void VkConnection::handleGotUnreadMessages ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		auto respList = data.toMap () ["response"].toMap () ["items"].toList ();
		if (respList.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no response"
					<< data;
			return;
		}
		respList.removeFirst ();

		QList<QPair<MessageInfo, FullMessageInfo>> infos;
		for (const auto& msgMapVar : respList)
		{
			const auto& map = msgMapVar.toMap ();
			if (map ["read_state"].toULongLong ())
				continue;

			Logger_ << "got unread message:" << QVariant { map };

			MessageFlags flags = MessageFlag::Unread;
			if (map ["out"].toULongLong ())
				flags |= MessageFlag::Outbox;

			MessageInfo info
			{
				map ["id"].toULongLong (),
				map ["user_id"].toULongLong (),
				map ["body"].toString (),
				flags,
				QDateTime::fromSecsSinceEpoch (map ["date"].toULongLong ()),
				{}
			};

			if (map.contains ("chat_id"))
			{
				info.Flags_ |= MessageFlag::Chat;
				info.Params_ ["from"] = info.From_;
				info.From_ = map ["chat_id"].toULongLong ();
			}

			infos.append ({ info, GetFullMessageInfo (map, Logger_) });
		}

		std::sort (infos.begin (), infos.end (),
				Util::ComparingBy ([] (const auto& pair) { return pair.first.TS_; }));
		for (const auto& pair : infos)
			emit gotMessage (pair.second, pair.first);
	}

	void VkConnection::handleChatCreated ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		auto info = Reply2ChatInfo_.take (reply);

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		info.ChatID_ = data.toMap () ["response"].toULongLong ();

		emit gotChatInfo (info);
	}

	void VkConnection::handleChatInfo ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		const auto& map = data.toMap () ["response"].toMap ();
		emit gotChatInfo ({
				map ["id"].toULongLong (),
				map ["title"].toString (),
				ParseUsers (map ["users"].toList ())
			});
	}

	void VkConnection::handleChatUserRemoved ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!CheckFinishedReply (reply))
			return;

		auto removeInfo = Reply2ChatRemoveInfo_.take (reply);

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		const auto& map = data.toMap ();
		if (map ["response"].toULongLong () == 1)
			emit chatUserRemoved (removeInfo.Chat_, removeInfo.User_);
	}

	void VkConnection::handleMessageSent ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		const auto& setter = MsgReply2Setter_.take (reply);
		if (!setter)
			return;

		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		const auto code = data.toMap ().value ("response", -1).toULongLong ();
		setter (code);
	}

	void VkConnection::handleCountriesFetched ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		const auto& setter = CountryReply2Setter_.take (reply);
		if (!setter)
			return;

		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		QHash<int, QString> result;
		for (const auto& item : data.toMap () ["response"].toList ())
		{
			const auto& map = item.toMap ();
			result [map ["id"].toInt ()] = map ["title"].toString ();
		}

		setter (result);
	}

	void VkConnection::handleMessageInfoFetched ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		const auto& setter = Reply2MessageSetter_.take (reply);
		if (!setter)
			return;

		if (!CheckFinishedReply (reply))
			return;

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO);
		try
		{
			CheckReplyData (data, reply);
		}
		catch (const CommandException&)
		{
			return;
		}

		Logger_ << "got message info data" << data;

		FullMessageInfo info;
		const auto& infoList = data.toMap () ["response"].toMap () ["items"].toList ();
		for (const auto& item : infoList)
		{
			if (item.type () != QVariant::Map)
				continue;

			info = GetFullMessageInfo (item.toMap (), Logger_);
		}

		setter (info);
	}

	void VkConnection::handleScopeSettingsChanged ()
	{
		AuthMgr_->UpdateScope (GetPerms ());
	}

	void VkConnection::saveCookies (const QByteArray& cookies)
	{
		LastCookies_ = cookies;
		emit cookiesChanged ();
	}
}
}
}
