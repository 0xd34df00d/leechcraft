/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QtKOAuth/QtKOAuth>
#include <interfaces/blasq/iaccount.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blasq
{
namespace Spegnersi
{
	class FlickrService;

	class FlickrAccount : public QObject
						, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IAccount)

		QString Name_;
		const QByteArray ID_;
		FlickrService * const Service_;
		const ICoreProxy_ptr Proxy_;

		KQOAuthRequest * const Req_;
		KQOAuthManager * const AuthMgr_;

		QStandardItemModel * const CollectionsModel_;
		QStandardItem *AllPhotosItem_ = 0;

		QString AuthToken_;
		QString AuthSecret_;

		bool UpdateAfterAuth_ = false;

		enum class State
		{
			Idle,
			AuthRequested,
			CollectionsRequested
		} State_ = State::Idle;

		QList<std::function<void ()>> CallQueue_;
	public:
		FlickrAccount (const QString&, FlickrService*, ICoreProxy_ptr, const QByteArray& = QByteArray ());

		QByteArray Serialize () const;
		static FlickrAccount* Deserialize (const QByteArray&, FlickrService*, ICoreProxy_ptr);

		QObject* GetQObject ();
		IService* GetService () const;
		QString GetName () const;
		QByteArray GetID () const;

		QAbstractItemModel* GetCollectionsModel () const;

		void UpdateCollections ();
	private:
		KQOAuthRequest* MakeRequest (const QUrl&, KQOAuthRequest::RequestType = KQOAuthRequest::AuthorizedRequest);

		void HandleCollectionsReply (const QByteArray&);
	private slots:
		void checkAuthTokens ();
		void requestTempToken ();

		void handleTempToken (const QString&, const QString&);
		void handleAuthorization (const QString&, const QString&);
		void handleAccessToken (const QString&, const QString&);

		void handleReply (const QByteArray&);
	signals:
		void accountChanged (FlickrAccount*);

		void doneUpdating ();
	};
}
}
}
