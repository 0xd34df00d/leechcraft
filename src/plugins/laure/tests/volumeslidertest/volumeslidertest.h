/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LAURE_VOLUMESLIDERTEST_H
#define PLUGINS_LAURE_VOLUMESLIDERTEST_H
#include <QObject>
#include <QtTest>

#include <volumeslider.h>

using namespace LeechCraft::Laure;

/** @test Provides test units for the VolumeSlider class.
 * 
 * @sa VolumeSlider
 * 
 *  @author Minh Ngo <nlminhtl@gmail.com>
 */
class VolumeSliderTest : public QObject
{
	Q_OBJECT
private slots:
	void setValue ()
	{
		VolumeSlider slider;
		slider.setValue (2);
		QVERIFY (slider.value () == 2);
	}
};

#endif // PLUGINS_LAURE_VOLUMESLIDERTEST_H