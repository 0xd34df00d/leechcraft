/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "screenshotsavedialog.h"
#include <algorithm>
#include <QImageWriter>
#include <QBuffer>
#include <QTimer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QtDebug>
#include <util/util.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ientityhandler.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
	ScreenShotSaveDialog::ScreenShotSaveDialog (const QPixmap& source,
			QWidget *parent)
	: QDialog (parent)
	, Source_ (source)
	, PixmapHolder_ (new QLabel ())
	{
		PixmapHolder_->setAlignment (Qt::AlignTop | Qt::AlignLeft);
		Ui_.setupUi (this);

		auto formats = QImageWriter::supportedImageFormats ();
		formats.removeAll ("ico");
		if (formats.contains ("jpg"))
			formats.removeAll ("jpeg");
		std::sort (formats.begin (), formats.end ());
		for (const auto& format : formats)
			Ui_.FormatCombobox_->addItem (format.toUpper ());
		if (formats.contains ("png"))
			Ui_.FormatCombobox_->setCurrentIndex (formats.indexOf ("png"));

		Ui_.Scroller_->setWidget (PixmapHolder_);
	}

	void ScreenShotSaveDialog::accept ()
	{
		const auto idx = Ui_.ActionBox_->currentIndex ();
		if (idx >= Filters_.size ())
			Save ();
		else
		{
			const auto& filter = Filters_.value (idx);
			auto e = Util::MakeEntity (QVariant::fromValue (Rendered_.toImage ()),
					{},
					{},
					"x-leechcraft/data-filter-request");

			e.Additional_ ["Format"] = Ui_.FormatCombobox_->currentText ();
			e.Additional_ ["Quality"] = Ui_.QualitySlider_->value ();

			e.Additional_ ["DataFilter"] = filter.ID_;

			auto ieh = qobject_cast<IEntityHandler*> (filter.Object_);
			ieh->Handle (e);
		}

		QDialog::accept ();
	}

	void ScreenShotSaveDialog::Save ()
	{
		const auto& defLoc = QStandardPaths::writableLocation (QStandardPaths::DocumentsLocation);
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Save screenshot"),
				XmlSettingsManager::Instance ()->Property ("ScreenshotsLocation",
					defLoc).toString ());
		if (filename.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("ScreenshotsLocation", filename);

		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not open %1 for write")
						.arg (filename));
			return;
		}

		const auto& format = Ui_.FormatCombobox_->currentText ();
		int quality = Ui_.QualitySlider_->value ();

		if (!Rendered_.save (&file, qPrintable (format), quality))
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not write screenshot to %1")
						.arg (filename));
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

	void ScreenShotSaveDialog::RepopulateActions ()
	{
		Ui_.ActionBox_->clear ();
		Filters_.clear ();

		const auto& imageVar = QVariant::fromValue (Rendered_.toImage ());

		auto proxy = Core::Instance ().GetProxy ();
		const auto& dfs = Util::GetDataFilters (imageVar,
				proxy->GetEntityManager ());
		for (auto df : dfs)
		{
			auto idf = qobject_cast<IDataFilter*> (df);
			for (const auto& var : idf->GetFilterVariants (imageVar))
			{
				Ui_.ActionBox_->addItem (var.Icon_, var.Name_);
				Filters_.append ({ df, var.ID_ });
			}
		}

		auto mgr = proxy->GetIconThemeManager ();
		Ui_.ActionBox_->addItem (mgr->GetIcon ("document-save"), tr ("Save"));
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

		RepopulateActions ();
	}

	void ScreenShotSaveDialog::on_QualitySlider__valueChanged ()
	{
		ScheduleRender ();
	}

	void ScreenShotSaveDialog::on_FormatCombobox__currentIndexChanged ()
	{
		ScheduleRender ();
	}
}
}
