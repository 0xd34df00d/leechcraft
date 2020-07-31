/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_singletrackerchanger.h"

namespace LC
{
namespace BitTorrent
{
	class SingleTrackerChanger : public QDialog
	{
		Q_OBJECT

		Ui::SingleTrackerChanger Ui_;
	public:
		SingleTrackerChanger (QWidget* = 0);

		void SetTracker (const QString&);
		void SetTier (int);
		QString GetTracker () const;
		int GetTier () const;
	};
}
}
