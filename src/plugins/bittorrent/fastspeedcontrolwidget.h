/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vector>
#include <QWidget>
#include "ui_fastspeedcontrolwidget.h"

namespace LC::BitTorrent
{
	class FastSpeedControlWidget : public QWidget
	{
		Q_OBJECT

		Ui::FastSpeedControlWidget Ui_;
		std::vector<std::pair<std::unique_ptr<QSpinBox>, std::unique_ptr<QSpinBox>>> Widgets_;
	public:
		explicit FastSpeedControlWidget (QWidget* = nullptr);
	private:
		void LoadSettings ();
		void SaveSettings ();
		void SetNum (int);
	public slots:
		void accept ();
		void reject ();
	signals:
		void speedsChanged ();
	};
}
