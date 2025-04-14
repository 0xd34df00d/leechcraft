/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcodingparamswidget.h"
#include <QThread>
#include <QtDebug>
#include "transcodingparams.h"
#include "formats.h"

namespace LC
{
namespace LMP
{
	TranscodingParamsWidget::TranscodingParamsWidget (QWidget *parent)
	: QWidget (parent)
	, Formats_ (new Formats)
	{
		Ui_.setupUi (this);

		const auto& fm = fontMetrics ();
		const auto qualityWidth = std::max (fm.horizontalAdvance (" 9999 kbps "),
				fm.horizontalAdvance (" " + tr ("Quality %1").arg (10) + " "));
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
			on_TranscodingFormat__currentIndexChanged ();
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
		return
		{
			Ui_.FilenameMask_->text (),
			transcode ? GetCurrentFormat ()->GetFormatID () : QString (),
			GetCurrentBitrateType (),
			Ui_.QualitySlider_->value (),
			Ui_.ThreadsSlider_->value (),
			Ui_.OnlyLosslessBox_->checkState () == Qt::Checked
		};
	}

	void TranscodingParamsWidget::SetParams (const TranscodingParams& params)
	{
		Ui_.TranscodingBox_->setChecked (!params.FormatID_.isEmpty ());
		Ui_.FilenameMask_->setText (params.FilePattern_);

		const auto& formats = Formats_->GetFormats ();
		const auto pos = std::find_if (formats.begin (), formats.end (),
				[params] (const Format_ptr& format)
					{ return format->GetFormatID () == params.FormatID_; });
		if (pos == formats.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown format"
					<< params.FormatID_
					<< "; available formats:";
			for (const auto& format : formats)
				qWarning () << "\t"
						<< format->GetFormatID ();
			return;
		}

		const auto idx = std::distance (formats.begin (), pos);
		Ui_.TranscodingFormat_->setCurrentIndex (idx);

		for (int i = 0; i < Ui_.BitrateTypeBox_->count (); ++i)
		{
			const auto& data = Ui_.BitrateTypeBox_->itemData (i);
			if (data.toInt () != static_cast<int> (params.BitrateType_))
				continue;

			Ui_.BitrateTypeBox_->setCurrentIndex (i);
			break;
		}

		Ui_.QualitySlider_->setValue (params.Quality_);

		Ui_.OnlyLosslessBox_->setCheckState (params.OnlyLossless_ ? Qt::Checked : Qt::Unchecked);
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

	void TranscodingParamsWidget::on_TranscodingFormat__currentIndexChanged ()
	{
		Ui_.BitrateTypeBox_->clear ();

		auto format = GetCurrentFormat ();
		if (!format)
			return;

		for (auto type : format->GetSupportedBitrates ())
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
