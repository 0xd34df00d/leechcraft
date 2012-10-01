/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "transcodingparamswidget.h"
#include <QThread>
#include "transcodingparams.h"
#include "formats.h"

namespace LeechCraft
{
namespace LMP
{
	TranscodingParamsWidget::TranscodingParamsWidget (QWidget *parent)
	: QWidget (parent)
	, Formats_ (new Formats)
	{
		Ui_.setupUi (this);
		const auto& fm = fontMetrics ();
		const auto qualityWidth = std::min (fm.width (" 9999 kbps "),
				fm.width (tr ("Quality %1").arg (10)));
		Ui_.QualityDisplay_->setFixedWidth (qualityWidth);
		Ui_.QualityDisplay_->setFrameShape (Ui_.ThreadsDisplay_->frameShape ());

		const int idealThreads = QThread::idealThreadCount ();
		if (idealThreads > 0)
		{
			Ui_.ThreadsSlider_->setMaximum (idealThreads * 2);
			Ui_.ThreadsSlider_->setValue (idealThreads > 1 ? idealThreads - 1 : 1);
		}
		else
			Ui_.ThreadsSlider_->setMaximum (4);

		const auto& formats = Formats_->GetFormats ();
		if (!formats.isEmpty ())
		{
			for (const auto& format : formats)
				Ui_.TranscodingFormat_->addItem (format->GetFormatName (), format->GetFormatID ());
			on_TranscodingFormat__currentIndexChanged (0);
			Ui_.StatusLabel_->hide ();
		}
		else
		{
			Ui_.TranscodingBox_->setChecked (false);
			Ui_.TranscodingBox_->setEnabled (false);

			Ui_.StatusLabel_->setText (tr ("No transcoding formats are available. Is ffmpeg installed?"));
		}
	}

	void TranscodingParamsWidget::SetMaskVisible (bool visible)
	{
		Ui_.FilenameMask_->setVisible (visible);
	}

	TranscodingParams TranscodingParamsWidget::GetParams () const
	{
		const bool transcode = Ui_.TranscodingBox_->isChecked ();
		const auto bitrateType = GetCurrentBitrateType ();
		return
		{
			Ui_.FilenameMask_->text (),
			transcode ? GetCurrentFormat ()->GetFormatID () : QString (),
			bitrateType,
			Ui_.QualitySlider_->value (),
			Ui_.ThreadsSlider_->value ()
		};
	}

	Format_ptr TranscodingParamsWidget::GetCurrentFormat () const
	{
		return Formats_->GetFormats ().value (Ui_.TranscodingFormat_->currentIndex ());
	}

	Format::BitrateType TranscodingParamsWidget::GetCurrentBitrateType () const
	{
		const auto& data = Ui_.BitrateTypeBox_->itemData (Ui_.BitrateTypeBox_->currentIndex ());
		return static_cast<Format::BitrateType> (data.toInt ());
	}

	void TranscodingParamsWidget::on_TranscodingFormat__currentIndexChanged (int idx)
	{
		Ui_.BitrateTypeBox_->clear ();

		auto format = GetCurrentFormat ();
		if (!format)
			return;

		Q_FOREACH (auto type, format->GetSupportedBitrates ())
			Ui_.BitrateTypeBox_->addItem (type == Format::BitrateType::CBR ? "CBR" : "VBR", static_cast<int> (type));

		on_BitrateTypeBox__currentIndexChanged (0);
	}

	void TranscodingParamsWidget::on_BitrateTypeBox__currentIndexChanged (int)
	{
		const auto& bitrate = GetCurrentBitrateType ();
		const auto& items = GetCurrentFormat ()->GetBitrateLabels (bitrate);

		const auto& value = items.size () == Ui_.QualitySlider_->maximum () + 1 ?
				Ui_.QualitySlider_->value () :
				items.size () / 2 + 1;

		Ui_.QualitySlider_->setMinimum (0);
		Ui_.QualitySlider_->setMaximum (items.size () - 1);
		Ui_.QualitySlider_->setValue (value);
		on_QualitySlider__valueChanged (value);
	}

	void TranscodingParamsWidget::on_QualitySlider__valueChanged (int pos)
	{
		const auto& bitrate = GetCurrentBitrateType ();
		const auto& items = GetCurrentFormat ()->GetBitrateLabels (bitrate);
		const auto& str = bitrate == Format::BitrateType::CBR ?
				tr ("%1 kbps").arg (items.value (pos)) :
				tr ("Quality %1").arg (items.value (pos));
				Ui_.QualityDisplay_->setText (str);
	}
}
}
