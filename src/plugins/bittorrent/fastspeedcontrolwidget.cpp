/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fastspeedcontrolwidget.h"
#include <QSettings>
#include <QHBoxLayout>
#include <util/util.h>

namespace LC::BitTorrent
{
	FastSpeedControlWidget::FastSpeedControlWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		LoadSettings ();

		connect (Ui_.Box_,
				&QSpinBox::valueChanged,
				this,
				&FastSpeedControlWidget::SetNum);
		connect (Ui_.Slider_,
				&QSlider::valueChanged,
				this,
				&FastSpeedControlWidget::SetNum);
	}

	namespace
	{
		static const QString GroupName = QStringLiteral ("FastSpeedControl");
		static const QString ArrayName = QStringLiteral ("Values");
		static const QString DownValue = QStringLiteral ("DownValue");
		static const QString UpValue = QStringLiteral ("UpValue");

		static constexpr int MinimumSpeed = 50;
	}

	void FastSpeedControlWidget::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup (GroupName);
		int num = settings.beginReadArray (ArrayName);
		if (!num)
			num = 1;
		Ui_.Box_->setValue (num);
		Ui_.Slider_->setValue (num);
		SetNum (num);

		int prev = MinimumSpeed;
		for (int i = 0; i < static_cast<int> (Widgets_.size ()); ++i)
		{
			settings.setArrayIndex (i);
			auto& [down, up] = Widgets_ [i];
			down->setValue (settings.value (DownValue, prev).toInt ());
			up->setValue (settings.value (UpValue, prev).toInt ());
			prev *= 3;
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void FastSpeedControlWidget::SaveSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup (GroupName);
		settings.remove ({});
		settings.beginWriteArray (ArrayName);
		for (int i = 0; i < static_cast<int> (Widgets_.size ()); ++i)
		{
			settings.setArrayIndex (i);
			auto& [down, up] = Widgets_ [i];
			settings.setValue (DownValue, down->value ());
			settings.setValue (UpValue, up->value ());
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void FastSpeedControlWidget::SetNum (int num)
	{
		while (static_cast<int> (Widgets_.size ()) < num)
		{
			const auto lay = new QHBoxLayout ();
			auto down = std::make_unique<QSpinBox> ();
			auto up = std::make_unique<QSpinBox> ();
			lay->addWidget (down.get ());
			lay->addWidget (up.get ());
			Ui_.Layout_->addLayout (lay);

			down->setSuffix (tr (" KiB/s"));
			up->setSuffix (tr (" KiB/s"));
			down->setRange (1, 1024 * 1024);
			up->setRange (1, 1024 * 1024);

			if (!Widgets_.empty ())
			{
				const auto& [prevDown, prevUp] = Widgets_.back ();
				down->setValue (prevDown->value () * 3);
				up->setValue (prevUp->value () * 3);
			}
			else
			{
				down->setValue (MinimumSpeed);
				up->setValue (MinimumSpeed);
			}

			Widgets_.emplace_back (std::move (down), std::move (up));
		}

		while (static_cast<int> (Widgets_.size ()) > num)
		{
			delete layout ()->takeAt (layout ()->count () - 1);
			Widgets_.pop_back ();
		}
	}

	void FastSpeedControlWidget::accept ()
	{
		SaveSettings ();

		emit speedsChanged ();
	}

	void FastSpeedControlWidget::reject ()
	{
		LoadSettings ();
	}
}
