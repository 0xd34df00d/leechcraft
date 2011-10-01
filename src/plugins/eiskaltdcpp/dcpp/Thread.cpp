/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "format.h"
#include "Thread.h"

namespace dcpp {

#ifndef _WIN32
pthread_mutex_t Thread::mtx = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef _WIN32
void Thread::start() throw(ThreadException) {
	join();
	if( (threadHandle = CreateThread(NULL, 0, &starter, this, 0, &threadId)) == NULL) {
		throw ThreadException(_("Unable to create thread"));
	}
}

#else
void Thread::start() throw(ThreadException) {
	join();
	if(pthread_create(&threadHandle, NULL, &starter, this) != 0) {
		throw ThreadException(_("Unable to create thread"));
	}
}
#endif

} // namespace dcpp
