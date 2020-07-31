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

namespace LC
{
namespace BitTorrent
{
	FastSpeedControlWidget::FastSpeedControlWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		LoadSettings ();
	}

	void FastSpeedControlWidget::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup ("FastSpeedControl");
		int num = settings.beginReadArray ("Values");
		if (!num)
			num = 1;
		Ui_.Box_->setValue (num);
		Ui_.Slider_->setValue (num);
		SetNum (num);

		int prev = 50;
		for (int i = 0; i < Widgets_.size (); ++i)
		{
			settings.setArrayIndex (i);
			Widgets_.at (i).first->
				setValue (settings.value ("DownValue", prev).toInt ());
			Widgets_.at (i).second->
				setValue (settings.value ("UpValue", prev).toInt ());
			prev *= 3;
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void FastSpeedControlWidget::SaveSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup ("FastSpeedControl");
		settings.remove ("");
		settings.beginWriteArray ("Values");
		for (int i = 0; i < Widgets_.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("DownValue", Widgets_.at (i).first->value ());
			settings.setValue ("UpValue", Widgets_.at (i).second->value ());
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void FastSpeedControlWidget::SetNum (int num)
	{
		while (Widgets_.size () < num)
		{
			QHBoxLayout *lay = new QHBoxLayout ();
			QSpinBox *down = new QSpinBox ();
			QSpinBox *up = new QSpinBox ();
			lay->addWidget (down);
			lay->addWidget (up);
			static_cast<QBoxLayout*> (layout ())->addLayout (lay);
			Widgets_ << qMakePair (down, up);

			down->setSuffix (tr (" KiB/s"));
			up->setSuffix (tr (" KiB/s"));
			down->setRange (1, 1024 * 1024);
			up->setRange (1, 1024 * 1024);

			if (Widgets_.size () > 1)
			{
				QPair<QSpinBox*, QSpinBox*> prev = Widgets_.at (Widgets_.size () - 2);
				down->setValue (prev.first->value () * 3);
				up->setValue (prev.second->value () * 3);
			}
			else
			{
				down->setValue (50);
				up->setValue (50);
			}
		}

		while (Widgets_.size () > num)
		{
			delete layout ()->takeAt (layout ()->count () - 1);

			QPair<QSpinBox*, QSpinBox*> pair = Widgets_.takeLast ();
			delete pair.first;
			delete pair.second;
		}
	}

	void FastSpeedControlWidget::on_Box__valueChanged (int val)
	{
		SetNum (val);
	}

	void FastSpeedControlWidget::on_Slider__valueChanged (int val)
	{
		SetNum (val);
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
}
