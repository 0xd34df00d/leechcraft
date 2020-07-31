/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>

class QSettings;

namespace LC
{
namespace CertMgr
{
	class ExceptionsModel : public QStandardItemModel
	{
		Q_OBJECT

		QSettings& Settings_;
	public:
		enum Column
		{
			Name,
			Status
		};

		enum Role
		{
			IsAllowed = Qt::UserRole + 1
		};

		ExceptionsModel (QSettings&, QObject*);

		void Populate ();

		void ToggleState (const QModelIndex&);
	private:
		void Add (const QString&, bool);
	};
}
}
