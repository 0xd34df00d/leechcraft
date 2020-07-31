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
		Q_OBJECT

		Ui::SelectGroupsDialog Ui_;
		QStandardItemModel * const Model_;
		QString Login_;
		FotoBilderAccount * const Account_;
	public:
		SelectGroupsDialog (const QString& login, FotoBilderAccount *acc,
				QWidget *parent = nullptr);

		uint GetSelectedGroupId () const;
	private:
		void RequestFriendsGroups ();
		void FriendsGroupsRequest (const QString& challenge);
		void GenerateChallenge ();
		QNetworkRequest CreateNetworkRequest ();
	private slots:
		void handleChallengeReplyFinished ();
		void handleNetworkError (QNetworkReply::NetworkError error);
		void handleRequestFriendsGroupsFinished ();
	};
}
}
}

Q_DECLARE_METATYPE (LC::Blasq::DeathNote::ParsedMember)
