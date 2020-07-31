/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_mooddialog.h"

namespace LC
{
namespace Azoth
{
	struct MoodInfo;

	class MoodDialog : public QDialog
	{
		Q_OBJECT

		Ui::MoodDialog Ui_;
	public:
		static QString ToHumanReadable (const QString&);

		MoodDialog (QWidget* = nullptr);
		
		MoodInfo GetMood () const;
		void SetMood (const MoodInfo&);
	private:
		static QMap<QString, QPair<QVariant, QIcon>> BuildHumanReadableList ();
	};
}
}
