/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "sensorsmanager.h"
#include <QTimer>
#include <QtDebug>

namespace LeechCraft
{
namespace HotSensors
{
	StoredChipName::StoredChipName ()
	: Addr_ (0)
	{
	}

	StoredChipName::StoredChipName (const sensors_chip_name *chipName)
	: Prefix_ (chipName->prefix)
	, Bus_ (chipName->bus)
	, Addr_ (chipName->addr)
	, Path_ (chipName->path)
	{
	}

	sensors_chip_name StoredChipName::ToSensorsChip ()
	{
		return { Prefix_.data (), Bus_, Addr_, Path_.data () };
	}

	SensorsManager::SensorsManager (QObject *parent)
	: QObject (parent)
	{
		sensors_init (nullptr);

		EnumerateSensors ();

		auto timer = new QTimer (this);
		timer->start (1000);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (readTemperatures ()));
		readTemperatures ();
	}

	SensorsManager::~SensorsManager ()
	{
		sensors_cleanup ();
	}

	void SensorsManager::EnumerateSensors ()
	{
		int nr = 0;
		const sensors_chip_name *chipName = 0;
		while ((chipName = sensors_get_detected_chips (nullptr, &nr)))
		{
			int fnr = 0;
			const sensors_feature *feature = 0;
			while ((feature = sensors_get_features (chipName, &fnr)))
			{
				if (feature->type != SENSORS_FEATURE_TEMP)
					continue;

				int sfnr = 0;
				const sensors_subfeature *subfeature = 0;

				StoredTemp temp;
				temp.Name_ = QString ("%1/%2")
						.arg (chipName->prefix)
						.arg (sensors_get_label (chipName, feature));
				while ((subfeature = sensors_get_all_subfeatures (chipName, feature, &sfnr)))
				{
					switch (subfeature->type)
					{
					case SENSORS_SUBFEATURE_TEMP_MAX:
						sensors_get_value (chipName, subfeature->number, &temp.Max_);
						break;
					case SENSORS_SUBFEATURE_TEMP_CRIT:
						sensors_get_value (chipName, subfeature->number, &temp.Crit_);
						break;
					case SENSORS_SUBFEATURE_TEMP_INPUT:
						temp.SF_ = StoredSubfeature { { chipName }, subfeature->number };
						break;
					default:
						break;
					}
				}
				Features_ << temp;
			}
		}
	}

	void SensorsManager::readTemperatures ()
	{
		for (auto feature : Features_)
		{
			const auto chipName = feature.SF_.Chip_.ToSensorsChip ();

			double value = 0;
			sensors_get_value (&chipName, feature.SF_.SF_, &value);
			qDebug () << feature.Name_ << value;
		}
	}
}
}
