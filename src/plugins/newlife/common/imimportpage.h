/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/structures.h>
#include "entitygeneratingpage.h"
#include "ui_imimportpage.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace NewLife
{
namespace Common
{
	class IMImportPage : public EntityGeneratingPage
	{
		Q_OBJECT
	protected:
		Ui::IMImportPage Ui_;

		QStandardItemModel *AccountsModel_;
	public:
		enum Roles
		{
			AccountData = Qt::UserRole + 1
		};

		enum Column
		{
			AccountName,
			JID,
			ImportAcc,
			ImportHist
		};

		IMImportPage (const ICoreProxy_ptr&, QWidget* = nullptr);

		bool isComplete () const override;
		int nextId () const override;
		void initializePage () override;
	protected:
		virtual void FindAccounts () = 0;
		virtual void SendImportAcc (QStandardItem*) = 0;
		virtual void SendImportHist (QStandardItem*) = 0;
	protected slots:
		virtual void handleAccepted ();
	};
}
}
}
