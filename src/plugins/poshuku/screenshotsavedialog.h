/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_SCREENSHOTSAVEDIALOG_H
#define PLUGINS_POSHUKU_SCREENSHOTSAVEDIALOG_H
#include <QDialog>
#include <QPixmap>
#include "ui_screenshotsavedialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class ScreenShotSaveDialog : public QDialog
			{
				Q_OBJECT

				Ui::ScreenShotSaveDialog Ui_;
				QPixmap Source_;
				mutable QPixmap Rendered_;
				mutable QLabel *PixmapHolder_;

				bool RenderScheduled_;
			public:
				ScreenShotSaveDialog (const QPixmap&, QWidget* = 0);
				QByteArray Save ();
			private:
				void ScheduleRender ();
			private slots:
				void render ();
				void on_QualitySlider__valueChanged ();
				void on_FormatCombobox__currentIndexChanged ();
			};
		};
	};
};

#endif

