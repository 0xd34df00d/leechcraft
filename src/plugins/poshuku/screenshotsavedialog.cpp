/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "screenshotsavedialog.h"
#include <algorithm>
#include <QImageWriter>
#include <QBuffer>
#include <QTimer>
#include <QtDebug>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			ScreenShotSaveDialog::ScreenShotSaveDialog (const QPixmap& source,
					QWidget *parent)
			: QDialog (parent)
			, Source_ (source)
			, PixmapHolder_ (new QLabel ())
			, RenderScheduled_ (false)
			{
				PixmapHolder_->setAlignment (Qt::AlignTop | Qt::AlignLeft);
				Ui_.setupUi (this);
			
				QList<QByteArray> formats = QImageWriter::supportedImageFormats ();
				formats.removeAll ("ico");
				if (formats.contains ("jpg"))
					formats.removeAll ("jpeg");
				std::sort (formats.begin (), formats.end ());
				for (QList<QByteArray>::const_iterator i = formats.begin (),
						end = formats.end (); i != end; ++i)
					Ui_.FormatCombobox_->addItem (i->toUpper ());
				if (formats.contains ("png"))
					Ui_.FormatCombobox_->setCurrentIndex (formats.indexOf ("png"));
			
				Ui_.Scroller_->setWidget (PixmapHolder_);
			}
			
			QByteArray ScreenShotSaveDialog::Save ()
			{
				QString format = Ui_.FormatCombobox_->currentText ();
				int quality = Ui_.QualitySlider_->value ();
			
				QBuffer renderBuffer;
				Source_.save (&renderBuffer, qPrintable (format), quality);
				return renderBuffer.data ();
			}
			
			void ScreenShotSaveDialog::ScheduleRender ()
			{
				if (RenderScheduled_)
					return;
			
				QTimer::singleShot (500,
						this,
						SLOT (render ()));
			
				RenderScheduled_ = true;
			}
			
			void ScreenShotSaveDialog::render ()
			{
				RenderScheduled_ = false;
				if (!Ui_.PreviewBox_->isChecked ())
				{
					Ui_.FileSizeLabel_->setText (tr ("File size unknown"));
					PixmapHolder_->setPixmap (QPixmap ());
					PixmapHolder_->resize (1, 1);
					return;
				}
			
				QString format = Ui_.FormatCombobox_->currentText ();
				int quality = Ui_.QualitySlider_->value ();
			
				QBuffer renderBuffer;
				Source_.save (&renderBuffer, qPrintable (format), quality);
				QByteArray renderData = renderBuffer.data ();
				Rendered_.loadFromData (renderData);
				Ui_.FileSizeLabel_->setText (Util::MakePrettySize (renderData.size ()));
				PixmapHolder_->setPixmap (Rendered_);
				PixmapHolder_->resize (Rendered_.size ());
			}
			
			void ScreenShotSaveDialog::on_QualitySlider__valueChanged ()
			{
				ScheduleRender ();
			}
			
			void ScreenShotSaveDialog::on_FormatCombobox__currentIndexChanged ()
			{
				ScheduleRender ();
			}
		};
	};
};

