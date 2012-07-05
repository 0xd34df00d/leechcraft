/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#pragma once

#include <lastfm/global.h>
#include <lastfm/misc.h>

#if LASTFM_MAJOR_VERSION < 1
#include <lastfm/ParseError>
#include <lastfm/Xspf>
#include <lastfm/MutableTrack>
#include <lastfm/Track>
#include <lastfm/ScrobbleCache>
#include <lastfm/Scrobble>
#include <lastfm/Audioscrobbler>
#include <lastfm/RadioTuner>
#include <lastfm/RadioStation>

#else

#include <lastfm/ParseError.h>
#include <lastfm/Xspf.h>
#include <lastfm/MutableTrack.h>
#include <lastfm/Track.h>
#include <lastfm/ScrobbleCache.h>
#include <lastfm/Scrobble.h>
#include <lastfm/Audioscrobbler.h>
#include <lastfm/RadioTuner.h>
#include <lastfm/RadioStation.h>

#endif
