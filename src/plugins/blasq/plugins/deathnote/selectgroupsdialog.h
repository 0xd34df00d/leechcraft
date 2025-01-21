/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <util/threads/coro/taskfwd.h>
#include "ui_selectgroupsdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	class FotoBilderAccount;

	struct FriendsGroup
	{
		bool Public_ = false;
		QString Name_;
		uint Id_ = 0;
		uint SortOrder_ = 0;
		uint RealId_ = 0;
	};

	struct ParsedMember
	{
		QString Name_;
		QVariantList Value_;
	};

	class SelectGroupsDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Blasq::DeathNote::SelectGroupsDialog)

		Ui::SelectGroupsDialog Ui_;
		QStandardItemModel * const Model_;
		QString Login_;
		FotoBilderAccount * const Account_;
	public:
		SelectGroupsDialog (const QString& login, FotoBilderAccount *acc,
				QWidget *parent = nullptr);

		uint GetSelectedGroupId () const;
	private:
		Util::ContextTask<> RequestFriendsGroups ();
		Util::ContextTask<> FriendsGroupsRequest (const QString& challenge);
		QNetworkRequest CreateNetworkRequest ();
		QByteArray GetFriendsGroupsRequestBody (const QString& challenge);
	};
}
}
}

Q_DECLARE_METATYPE (LC::Blasq::DeathNote::ParsedMember)
