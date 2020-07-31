/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "speedselectoraction.h"
#include <functional>
#include <QComboBox>
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
namespace BitTorrent
{
	SpeedSelectorAction::SpeedSelectorAction (const QString& s, QObject *parent)
	: QWidgetAction (parent)
	, Setting_ (s)
	{
	}

	int SpeedSelectorAction::CurrentData ()
	{
		QList<QWidget*> ws = createdWidgets ();
		if (ws.size ())
		{
			QComboBox *bx = static_cast<QComboBox*> (ws.at (0));
			return bx->itemData (bx->currentIndex ()).toInt ();
		}
		else
			return 0;
	}

	QWidget* SpeedSelectorAction::createWidget (QWidget *parent)
	{
		QComboBox *selector = new QComboBox (parent);
		connect (selector,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (syncSpeeds (int)));

		QTimer::singleShot (0,
				this,
				SLOT (handleSpeedsChanged ()));

		return selector;
	}

	void SpeedSelectorAction::deleteWidget (QWidget *w)
	{
		delete w;
	}

	void SpeedSelectorAction::handleSpeedsChanged ()
	{
		Call ([] (QComboBox *box) { box->clear (); });

		if (!XmlSettingsManager::Instance ()->
				property ("EnableFastSpeedControl").toBool ())
		{
			setVisible (false);
			return;
		}

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup ("FastSpeedControl");
		int num = settings.beginReadArray ("Values");
		for (int i = 0; i < num; ++i)
		{
			settings.setArrayIndex (i);
			int dv = settings.value (Setting_ + "Value").toInt ();
			const auto& str = tr ("%1 KiB/s").arg (dv);
			Call ([dv, str] (QComboBox *box) { box->addItem (str, dv); });
		}
		settings.endArray ();
		settings.endGroup ();

		Call ([] (QComboBox *box) { box->addItem (QString::fromUtf8 ("\u221E"), 0); });
		Call ([] (QComboBox *box) { box->setCurrentIndex (box->count () - 1); });

		setVisible (true);
	}

	void SpeedSelectorAction::syncSpeeds (int s)
	{
		Call ([s] (QComboBox *box) { box->setCurrentIndex (s); });
		emit currentIndexChanged (s);
	}

	template<typename F>
	void SpeedSelectorAction::Call (F&& f)
	{
		for (const auto w : createdWidgets ())
			f (static_cast<QComboBox*> (w));
	}
}
}
