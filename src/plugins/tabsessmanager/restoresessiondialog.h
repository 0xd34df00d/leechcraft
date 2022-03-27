/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_restoresessiondialog.h"
#include "recinfo.h"

namespace LC
{
namespace TabSessManager
{
	class RestoreSessionDialog : public QDialog
	{
		Q_OBJECT

		Ui::RestoreSessionDialog Ui_;
	public:
		RestoreSessionDialog (QWidget* = 0);

		void SetTabs (const QHash<QObject*, QList<RecInfo>>&);
		QHash<QObject*, QList<RecInfo>> GetTabs () const;
	private:
		void CheckAll (Qt::CheckState);
	};
}
}
