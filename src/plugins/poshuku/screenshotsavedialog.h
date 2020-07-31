/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QPixmap>
#include "ui_screenshotsavedialog.h"

namespace LC
{
namespace Poshuku
{
	class ScreenShotSaveDialog : public QDialog
	{
		Q_OBJECT

		Ui::ScreenShotSaveDialog Ui_;

		const QPixmap Source_;
		QPixmap Rendered_;
		QLabel *PixmapHolder_;

		bool RenderScheduled_ = false;

		struct FilterData
		{
			QObject *Object_;
			QByteArray ID_;
		};
		QList<FilterData> Filters_;
	public:
		ScreenShotSaveDialog (const QPixmap&, QWidget* = 0);

		void accept () override;
	private:
		void Save ();

		void ScheduleRender ();

		void RepopulateActions ();
	private slots:
		void render ();
		void on_QualitySlider__valueChanged ();
		void on_FormatCombobox__currentIndexChanged ();
	};
}
}
