/*
 * idle.cpp - detect desktop idle time
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "idle.h"

#include <qcursor.h>
#include <qdatetime.h>
#include <qtimer.h>

static IdlePlatform *platform = 0;
static int platform_ref = 0;

class Idle::Private
{
public:
	Private() {}

	QPoint lastMousePos;
	QDateTime idleSince;

	bool active;
	int idleTime;
	QTimer checkTimer;
};

Idle::Idle()
{
	d = new Private;
	d->active = false;
	d->idleTime = 0;

	// try to use platform idle
	if(!platform) {
		IdlePlatform *p = new IdlePlatform;
		if(p->init())
			platform = p;
		else
			delete p;
	}
	if(platform)
		++platform_ref;

	connect(&d->checkTimer, SIGNAL(timeout()), SLOT(doCheck()));
}

Idle::~Idle()
{
	if(platform) {
		--platform_ref;
		if(platform_ref == 0) {
			delete platform;
			platform = 0;
		}
	}
	delete d;
}

bool Idle::isActive() const
{
	return d->active;
}

bool Idle::usingPlatform() const
{
	return (platform ? true: false);
}

void Idle::start()
{
	if(!platform) {
		// generic idle
		d->lastMousePos = QCursor::pos();
		d->idleSince = QDateTime::currentDateTime();
	}

	// poll every second (use a lower value if you need more accuracy)
	d->checkTimer.start(10000);
}

void Idle::stop()
{
	d->checkTimer.stop();
}

int Idle::interval() const
{
	return d->checkTimer.interval();
}

void Idle::doCheck()
{
	int i = 0;
	if(platform)
		i = platform->secondsIdle();
	else {
		QPoint curMousePos = QCursor::pos();
		QDateTime curDateTime = QDateTime::currentDateTime();
		if(d->lastMousePos != curMousePos) {
			d->lastMousePos = curMousePos;
			d->idleSince = curDateTime;
		}
		i = d->idleSince.secsTo(curDateTime);
	}
	secondsIdle(i);
}
