/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/data/iimgsource.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Blasq
{
	class AccountsManager;
	class PhotosTab;

	class DefaultImageChooser : public QObject
							  , public IPendingImgSourceRequest
	{
		Q_OBJECT
		Q_INTERFACES (IPendingImgSourceRequest)

		AccountsManager * const AccMgr_;
		const ICoreProxy_ptr Proxy_;

		PhotosTab * const Photos_;

		RemoteImageInfos_t Selected_;
	public:
		DefaultImageChooser (AccountsManager*, const ICoreProxy_ptr&, const QByteArray& = {});

		QObject* GetQObject ();
		RemoteImageInfos_t GetInfos () const;
	private slots:
		void handleAccept ();
		void handleReject ();
	signals:
		void ready ();
		void error (const QString&);
	};
}
}
