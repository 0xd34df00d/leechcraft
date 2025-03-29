/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "speedselectoraction.h"
#include <QComboBox>
#include <QSettings>
#include "xmlsettingsmanager.h"

namespace LC::BitTorrent
{
	SpeedSelectorAction::SpeedSelectorAction (SessionSettingsManager *ssm,
			Setter_t setter,
			const QString& s,
			QObject *parent)
	: QWidgetAction { parent }
	, SSM_ { ssm }
	, Setter_ { setter }
	, Setting_ { s }
	{
	}

	namespace
	{
		QList<int> GetSpeeds (const QString& setting)
		{
			QList<int> result;

			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Torrent");
			settings.beginGroup (QStringLiteral ("FastSpeedControl"));
			int num = settings.beginReadArray (QStringLiteral ("Values"));
			for (int i = 0; i < num; ++i)
			{
				settings.setArrayIndex (i);
				result << settings.value (setting + "Value").toInt ();
			}
			settings.endArray ();
			settings.endGroup ();

			return result;
		}

		void UpdateSpeeds (QComboBox *selector, const QList<int>& speeds)
		{
			selector->clear ();
			for (const auto dv : speeds)
				selector->addItem (SpeedSelectorAction::tr ("%1 KiB/s").arg (dv), dv);
		}
	}

	void SpeedSelectorAction::HandleSpeedsChanged ()
	{
		if (!XmlSettingsManager::Instance ().property ("EnableFastSpeedControl").toBool ())
		{
			setVisible (false);
			return;
		}

		const auto& speeds = GetSpeeds (Setting_);
		for (const auto box : Boxes_)
			UpdateSpeeds (box, speeds);

		setVisible (true);
	}

	QWidget* SpeedSelectorAction::createWidget (QWidget *parent)
	{
		const auto selector = new QComboBox { parent };
		connect (selector,
				&QComboBox::currentIndexChanged,
				this,
				[this] (int s)
				{
					for (const auto w : Boxes_)
						w->setCurrentIndex (s);
					(SSM_->*Setter_) (s);
				});

		UpdateSpeeds (selector, GetSpeeds (Setting_));

		return selector;
	}

	void SpeedSelectorAction::deleteWidget (QWidget *w)
	{
		Boxes_.removeOne (static_cast<QComboBox*> (w));
		delete w;
	}
}
