/* Copyright (C) 2006 - 2011 Jan Kundrát <jkt@gentoo.org>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TROJITA_GUI_ICONLOADER_H
#define TROJITA_GUI_ICONLOADER_H

#include <QIcon>
#include "core.h"

namespace Gui {

/** @short Wrapper around the QIcon::fromTheme with sane fallback
 *
 * The issue with the QIcon::fromTheme is that it does not realy work on non-X11
 * platforms, unless you ship your own theme index and specify your theme name.
 * In Trojitá, we do not really want to hardcode anything, because the
 * preference is to use whatever is available from the current theme, but *also*
 * provide an icon as a fallback. And that is exactly why this function is here.
 *
 * It is implemented inline in order to prevent a dependency from the Imap lib
 * into the Gui lib.
 * */
inline QIcon loadIcon(const QString &name)
{
    return LeechCraft::Snails::Core::Instance ().GetProxy ()->GetIcon (name);
}

}

#endif // TROJITA_GUI_ICONLOADER_H
