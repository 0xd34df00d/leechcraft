/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/azoth/imucjoinwidget.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_mucjoinwidget.h"

class QStandardItemModel;
class QSortFilterProxyModel;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkEntry;

	class MucJoinWidget : public QWidget
						, public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCJoinWidget)

		Ui::MucJoinWidget Ui_;
		QStandardItemModel *UsersModel_;
		QSortFilterProxyModel *UsersFilter_;
	public:
		MucJoinWidget (ICoreProxy_ptr);

		void AccountSelected (QObject*);
		void Join (QObject*);
		void Cancel ();

		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap& data);
	private:
		QList<VkEntry*> GetSelectedEntries () const;
	signals:
		void validityChanged (bool);
	};
}
}
}
