/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_LAURE_VLCWRAPPERTEST_H
#define PLUGINS_LAURE_VLCWRAPPERTEST_H
#include <QObject>
#include <QtTest>

#include <vlcwrapper.h>

using namespace LeechCraft;
using namespace LeechCraft::Laure;

/** @test Provides test units for the VLCWrapper class.
 * 
 * Needs internet connection for playing the media file.
 * 
 * @sa VLCWrapper
 * 
 * @author Minh Ngo <nlminhtl@gmail.com>
 */
class VLCWrapperTest : public QObject
{
	Q_OBJECT
	
	VLCWrapper *Wrapper_;
public:
	VLCWrapperTest (QObject *parent = 0)
	: Wrapper_ (new VLCWrapper (this))
	{
	}
private slots:
	void initTest ()
	{
		qRegisterMetaType<MediaMeta> ("MediaMeta");
		qRegisterMetaType<Entity> ("Entity");
		QSignalSpy addSpy (Wrapper_, SIGNAL (itemAdded (const MediaMeta&, const QString&)));
		
		const QString url ("http://www.gnu.org/music/FreeSWSong.ogg"); 
		Wrapper_->addRow (url);
		
		QVERIFY (Wrapper_->RowCount () == 1);
		
		QCOMPARE (addSpy.count (), 1);
		QVERIFY (addSpy.takeFirst ().at (1).toString () == url);
		
		QSignalSpy entitySpy (Wrapper_, SIGNAL (gotEntity (const Entity&)));
		QSignalSpy playedSpy (Wrapper_, SIGNAL (itemPlayed (int)));
		
		Wrapper_->playItem (0);
		
		QCOMPARE (entitySpy.count (), 1);
		QCOMPARE (playedSpy.count (), 1);
		
		QTimer::singleShot (5000, this, SLOT (playTest ()));
	}

	void playTest ()
	{
		QVERIFY (Wrapper_->IsPlaying ());
		
		QSignalSpy pauseSpy (Wrapper_, SIGNAL (paused ()));
		
		Wrapper_->play ();
		
		QCOMPARE (pauseSpy.count (), 0);
	}
	
	void volumeTest ()
	{
		Wrapper_->setVolume (50);
		QVERIFY (Wrapper_->GetVolume () == 50);
	}
};


#endif // PLUGINS_LAURE_VLCWRAPPERTEST_H
