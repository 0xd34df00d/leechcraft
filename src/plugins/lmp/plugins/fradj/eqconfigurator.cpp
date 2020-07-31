/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eqconfigurator.h"
#include <algorithm>
#include <iterator>
#include <QSettings>
#include "eqconfiguratordialog.h"
#include "iequalizer.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	EqConfigurator::EqConfigurator (IEqualizer *eq, QObject *parent)
	: QObject { parent }
	, IEq_ { eq }
	, ID_ { IEq_->GetEffectId () }
	, Bands_ { IEq_->GetFixedBands () }
	{
	}

	void EqConfigurator::Restore ()
	{
		IEq_->SetGains (ReadGains ());
	}

	void EqConfigurator::OpenDialog ()
	{
		const auto& gains = ReadGains ();

		EqConfiguratorDialog dia { Bands_, gains, IEq_->GetPresets () };
		connect (&dia,
				&EqConfiguratorDialog::presetRequested,
				[this, &dia] (const QString& preset)
				{
					if (!preset.isEmpty ())
					{
						IEq_->SetPreset (preset);
						dia.SetGains (IEq_->GetGains ());
					}
				});
		const auto isAccepted = dia.exec () == QDialog::Accepted;

		const auto& newGains = isAccepted ? dia.GetGains () : gains;

		IEq_->SetGains (newGains);

		SaveGains (newGains);
	}

	QList<double> EqConfigurator::ReadGains () const
	{
		QList<double> gains;

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Fradj" };
		settings.beginGroup (ID_);
		const int count = settings.beginReadArray ("Gains");

		if (count)
			for (int i = 0; i < count; ++i)
			{
				settings.setArrayIndex (i);
				gains << settings.value ("Gain").toDouble ();
			}
		else
			std::fill_n (std::back_inserter (gains), Bands_.size (), 0);

		settings.endArray ();
		settings.endGroup ();

		return gains;
	}

	void EqConfigurator::SaveGains (const QList<double>& gains) const
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Fradj" };
		settings.beginGroup (ID_);
		settings.beginWriteArray ("Gains");
		for (int i = 0; i < gains.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("Gain", gains.at (i));
		}
		settings.endArray ();
		settings.endGroup ();
	}
}
}
}
