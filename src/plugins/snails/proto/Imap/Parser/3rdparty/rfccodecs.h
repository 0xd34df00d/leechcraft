/**********************************************************************
 *
 *   rfccodecs  - handler for various rfc/mime encodings
 *   Copyright (C) 2000 s.carstens@gmx.de
 *
 *   Imported from KDE's SVN and stripped of various parts which are not
 *   used in Trojita. Please see KDE SVN or Trojita's git repo for the
 *   original version.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 *
 *********************************************************************/
/**
 * @file
 * This file is part of the IMAP support library and defines the
 * RfcCodecs class.
 *
 * @brief
 * Provides handlers for various RFC/MIME encodings.
 *
 * @author Sven Carstens
 */

#ifndef KIMAP_RFCCODECS_H
#define KIMAP_RFCCODECS_H

#include <QtCore/QString>

#define KIMAP_EXPORT

class QTextCodec;

namespace KIMAP {

  /**
    Converts an Unicode IMAP mailbox to a QString which can be used in
    IMAP communication.
    @param src is the QString containing the IMAP mailbox.
  */
  KIMAP_EXPORT QString encodeImapFolderName( const QString &src );

  /**
    Converts an UTF-7 encoded IMAP mailbox to a Unicode QString.
    @param inSrc is the QString containing the Unicode path.
  */
  KIMAP_EXPORT QString decodeImapFolderName( const QString &inSrc );

  /**
    Replaces " with \" and \ with \\ " and \ characters.
    @param src is the QString to quote.
  */
  KIMAP_EXPORT QString quoteIMAP( const QString &src );
}

#endif
