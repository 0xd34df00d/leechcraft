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

namespace LC::BitTorrent
{
	class SingleTrackerChanger : public QDialog
	{
		Ui::SingleTrackerChanger Ui_;
	public:
		explicit SingleTrackerChanger (QWidget* = nullptr);

		void SetTracker (const QString&);
		void SetTier (int);
		QString GetTracker () const;
		int GetTier () const;
	};
}
