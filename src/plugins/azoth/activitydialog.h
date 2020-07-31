/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_activitydialog.h"

namespace LC
{
namespace Azoth
{
	struct ActivityInfo;

	class ActivityDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::ActivityDialog Ui_;
		QMap<QString, QStringList> Gen2Specific_;

		static auto GetActivityInfos ();
	public:
		static QString ToHumanReadable (const QString&);

		ActivityDialog (QWidget* = nullptr);

		ActivityInfo GetActivityInfo () const;
		void SetActivityInfo (const ActivityInfo&);
	private:
		void SetCurrentActivityItem (const ActivityInfo&);
	};
}
}
