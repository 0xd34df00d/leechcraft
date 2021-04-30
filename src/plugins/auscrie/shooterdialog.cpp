/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shooterdialog.h"
#include <QPushButton>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/idatafilter.h>
#include <interfaces/iinfo.h>
#include "util.h"

namespace LC::Auscrie
{
	ShooterDialog::ShooterDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		on_Format__currentIndexChanged (Ui_.Format_->currentText ());

		auto button = new QPushButton (tr ("Make screenshot"));
		Ui_.ButtonBox_->addButton (button, QDialogButtonBox::ApplyRole);
		connect (button,
				SIGNAL (released ()),
				this,
				SIGNAL (screenshotRequested ()));
	}

	ShooterDialog::Action ShooterDialog::GetAction () const
	{
		return Ui_.ActionBox_->currentIndex () == Ui_.ActionBox_->count () - 1 ?
				Action::Save :
				Action::Upload;
	}

	ShooterDialog::Mode ShooterDialog::GetMode () const
	{
		switch (Ui_.ModeBox_->currentIndex ())
		{
		case 0:
			return Mode::LCWindowOverlay;
		case 1:
			return Mode::LCWindow;
		case 2:
			return Mode::CurrentScreen;
		case 3:
			return Mode::WholeDesktop;
		default:
			qWarning () << Q_FUNC_INFO
					<< Ui_.ModeBox_->currentIndex ()
					<< "unhandled";
			return Mode::LCWindowOverlay;
		}
	}

	bool ShooterDialog::ShouldHide () const
	{
		return Ui_.HideThis_->checkState () == Qt::Checked;
	}

	int ShooterDialog::GetTimeout () const
	{
		return Ui_.Timeout_->value ();
	}

	QString ShooterDialog::GetFormat () const
	{
		return Ui_.Format_->currentText ();
	}

	int ShooterDialog::GetQuality () const
	{
		const int val = Ui_.QualityBox_->value ();
		return Ui_.Format_->currentText () == "JPG" ?
				val :
				Ui_.QualityBox_->maximum () - val;
	}

	ShooterDialog::FilterData ShooterDialog::GetDFInfo () const
	{
		return Filters_.value (Ui_.ActionBox_->currentIndex ());
	}

	void ShooterDialog::SetScreenshot (const QPixmap& px)
	{
		CurrentScreenshot_ = px;
		RescaleLabel ();

		Ui_.ActionBox_->clear ();
		Filters_.clear ();

		const auto& selected = RestoreFilterState ();

		const auto& imageVar = QVariant::fromValue (px.toImage ());
		const auto& filters = Util::GetDataFilters (imageVar, GetProxyHolder ()->GetEntityManager ());
		for (auto filter : filters)
		{
			auto idf = qobject_cast<IDataFilter*> (filter);
			const auto& verb = idf->GetFilterVerb ();

			const auto& pluginId = qobject_cast<IInfo*> (filter)->GetUniqueID ();

			for (const auto& var : idf->GetFilterVariants (imageVar))
			{
				Filters_.append ({ filter, var.ID_ });
				Ui_.ActionBox_->addItem (QStringLiteral ("%1: %2").arg (verb, var.Name_));

				if (pluginId == selected.PluginId_ &&
						var.ID_ == selected.Variant_)
					Ui_.ActionBox_->setCurrentIndex (Ui_.ActionBox_->count () - 1);
			}
		}
		Ui_.ActionBox_->addItem (tr ("save"));
	}

	QPixmap ShooterDialog::GetScreenshot () const
	{
		return CurrentScreenshot_;
	}

	void ShooterDialog::resizeEvent (QResizeEvent *e)
	{
		QDialog::resizeEvent (e);

		RescaleLabel ();
	}

	void ShooterDialog::RescaleLabel ()
	{
		const auto& scaled = CurrentScreenshot_.scaled (Ui_.ScreenshotLabel_->size (),
				Qt::KeepAspectRatio, Qt::SmoothTransformation);
		Ui_.ScreenshotLabel_->setPixmap (scaled);
	}

	void ShooterDialog::on_Format__currentIndexChanged (const QString& str)
	{
		if (str == "JPG"_ql)
			Ui_.SettingLabel_->setText (tr ("Quality:"));
		else
			Ui_.SettingLabel_->setText (tr ("Compression:"));
	}
}
