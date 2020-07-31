/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/structures.h>

class QUrl;

namespace LC
{
namespace Blasq
{
	class AccountsManager;
	struct UploadItem;

	class DataFilterUploader : public QObject
	{
		Q_OBJECT

		AccountsManager * const AccMgr_;
		const Entity Entity_;
		QString UploadFileName_;
	public:
		DataFilterUploader (const Entity&, AccountsManager*, QObject* = nullptr);
	private:
		void SelectAcc ();
		void UploadToAcc (const QByteArray&);
	private slots:
		void checkItemUploaded (const UploadItem&, const QUrl&);
	};
}
}
