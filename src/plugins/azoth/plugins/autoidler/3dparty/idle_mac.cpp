/*
 * idle_mac.cpp - detect desktop idle time
 * Copyright (C) 2003  Tarkvara Design Inc.
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
#include <ApplicationServices/ApplicationServices.h>

IdlePlatform::IdlePlatform() {
}

IdlePlatform::~IdlePlatform() {
}

bool IdlePlatform::init() {
	return true;
}


int IdlePlatform::secondsIdle() {
	return CGEventSourceSecondsSinceLastEventType (kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
}
