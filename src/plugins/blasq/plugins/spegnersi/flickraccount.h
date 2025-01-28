/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/threads/coro/taskfwd.h>
#include <interfaces/blasq/iaccount.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;
class QStandardItem;

class QOAuth1;

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

		QOAuth1 * const AuthMgr_;

		QStandardItemModel * const CollectionsModel_;
		QStandardItem *AllPhotosItem_ = nullptr;
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
		void HandleCollectionsReply (const QByteArray&);
		Util::ContextTask<> UpdateCollectionsPage (std::optional<int>);
	signals:
		void accountChanged (FlickrAccount*);

		void doneUpdating ();
	};
}
}
}
