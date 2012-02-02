/**********************************************************************
 *
 *   rfccodecs.cpp - handler for various rfc/mime encodings
 *   Copyright (C) 2000 s.carstens@gmx.de
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
 * Defines the RfcCodecs class.
 *
 * @author Sven Carstens
 */

#include "rfccodecs.h"

#include <ctype.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QTextCodec>
#include <QtCore/QBuffer>
#include <QtCore/QRegExp>
#include <QtCore/QByteArray>
#include <QtCore/QLatin1Char>
#include "kcodecs.h"

using namespace KIMAP;

// This part taken from rfc 2192 IMAP URL Scheme. C. Newman. September 1997.
// adapted to QT-Toolkit by Sven Carstens <s.carstens@gmx.de> 2000

//@cond PRIVATE
static const unsigned char base64chars[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";
#define UNDEFINED 64
#define MAXLINE  76
static const char especials[17] = "()<>@,;:\"/[]?.= ";

/* UTF16 definitions */
#define UTF16MASK       0x03FFUL
#define UTF16SHIFT      10
#define UTF16BASE       0x10000UL
#define UTF16HIGHSTART  0xD800UL
#define UTF16HIGHEND    0xDBFFUL
#define UTF16LOSTART    0xDC00UL
#define UTF16LOEND      0xDFFFUL
//@endcond

//-----------------------------------------------------------------------------
QString KIMAP::decodeImapFolderName( const QString &inSrc )
{
  unsigned char c, i, bitcount;
  unsigned long ucs4, utf16, bitbuf;
  unsigned char base64[256], utf8[6];
  unsigned int srcPtr = 0;
  QByteArray dst;
  QByteArray src = inSrc.toAscii ();
  uint srcLen = inSrc.length();

  /* initialize modified base64 decoding table */
  memset( base64, UNDEFINED, sizeof( base64 ) );
  for ( i = 0; i < sizeof( base64chars ); ++i ) {
    base64[(int)base64chars[i]] = i;
  }

  /* loop until end of string */
  while ( srcPtr < srcLen ) {
    c = src[srcPtr++];
    /* deal with literal characters and &- */
    if ( c != '&' || src[srcPtr] == '-' ) {
      /* encode literally */
      dst += c;
      /* skip over the '-' if this is an &- sequence */
      if ( c == '&' ) {
        srcPtr++;
      }
    } else {
      /* convert modified UTF-7 -> UTF-16 -> UCS-4 -> UTF-8 -> HEX */
      bitbuf = 0;
      bitcount = 0;
      ucs4 = 0;
      while ( ( c = base64[(unsigned char)src[srcPtr]] ) != UNDEFINED ) {
        ++srcPtr;
        bitbuf = ( bitbuf << 6 ) | c;
        bitcount += 6;
        /* enough bits for a UTF-16 character? */
        if ( bitcount >= 16 ) {
          bitcount -= 16;
          utf16 = ( bitcount ? bitbuf >> bitcount : bitbuf ) & 0xffff;
          /* convert UTF16 to UCS4 */
          if ( utf16 >= UTF16HIGHSTART && utf16 <= UTF16HIGHEND ) {
            ucs4 = ( utf16 - UTF16HIGHSTART ) << UTF16SHIFT;
            continue;
          } else if ( utf16 >= UTF16LOSTART && utf16 <= UTF16LOEND ) {
            ucs4 += utf16 - UTF16LOSTART + UTF16BASE;
          } else {
            ucs4 = utf16;
          }
          /* convert UTF-16 range of UCS4 to UTF-8 */
          if ( ucs4 <= 0x7fUL ) {
            utf8[0] = ucs4;
            i = 1;
          } else if ( ucs4 <= 0x7ffUL ) {
            utf8[0] = 0xc0 | ( ucs4 >> 6 );
            utf8[1] = 0x80 | ( ucs4 & 0x3f );
            i = 2;
          } else if ( ucs4 <= 0xffffUL ) {
            utf8[0] = 0xe0 | ( ucs4 >> 12 );
            utf8[1] = 0x80 | ( ( ucs4 >> 6 ) & 0x3f );
            utf8[2] = 0x80 | ( ucs4 & 0x3f );
            i = 3;
          } else {
            utf8[0] = 0xf0 | ( ucs4 >> 18 );
            utf8[1] = 0x80 | ( ( ucs4 >> 12 ) & 0x3f );
            utf8[2] = 0x80 | ( ( ucs4 >> 6 ) & 0x3f );
            utf8[3] = 0x80 | ( ucs4 & 0x3f );
            i = 4;
          }
          /* copy it */
          for ( c = 0; c < i; ++c ) {
            dst += utf8[c];
          }
        }
      }
      /* skip over trailing '-' in modified UTF-7 encoding */
      if ( src[srcPtr] == '-' ) {
        ++srcPtr;
      }
    }
  }
  return QString::fromUtf8( dst.data () );
}

//-----------------------------------------------------------------------------
QString KIMAP::quoteIMAP( const QString &src )
{
  uint len = src.length();
  QString result;
  result.reserve( 2 * len );
  for ( unsigned int i = 0; i < len; i++ ) {
    if ( src[i] == '"' || src[i] == '\\' ) {
      result += '\\';
    }
    result += src[i];
  }
  //result.squeeze(); - unnecessary and slow
  return result;
}

//-----------------------------------------------------------------------------
QString KIMAP::encodeImapFolderName( const QString &inSrc )
{
  unsigned int utf8pos, utf8total, c, utf7mode, bitstogo, utf16flag;
  unsigned int ucs4, bitbuf;
  QByteArray src = inSrc.toUtf8 ();
  QString dst;

  int srcPtr = 0;
  utf7mode = 0;
  utf8total = 0;
  bitstogo = 0;
  utf8pos = 0;
  bitbuf = 0;
  ucs4 = 0;
  while ( srcPtr < src.length () ) {
    c = (unsigned char)src[srcPtr++];
    /* normal character? */
    if ( c >= ' ' && c <= '~' ) {
      /* switch out of UTF-7 mode */
      if ( utf7mode ) {
        if ( bitstogo ) {
          dst += base64chars[( bitbuf << ( 6 - bitstogo ) ) & 0x3F];
          bitstogo = 0;
        }
        dst += '-';
        utf7mode = 0;
      }
      dst += c;
      /* encode '&' as '&-' */
      if ( c == '&' ) {
        dst += '-';
      }
      continue;
    }
    /* switch to UTF-7 mode */
    if ( !utf7mode ) {
      dst += '&';
      utf7mode = 1;
    }
    /* Encode US-ASCII characters as themselves */
    if ( c < 0x80 ) {
      ucs4 = c;
      utf8total = 1;
    } else if ( utf8total ) {
      /* save UTF8 bits into UCS4 */
      ucs4 = ( ucs4 << 6 ) | ( c & 0x3FUL );
      if ( ++utf8pos < utf8total ) {
        continue;
      }
    } else {
      utf8pos = 1;
      if ( c < 0xE0 ) {
        utf8total = 2;
        ucs4 = c & 0x1F;
      } else if ( c < 0xF0 ) {
        utf8total = 3;
        ucs4 = c & 0x0F;
      } else {
        /* NOTE: can't convert UTF8 sequences longer than 4 */
        utf8total = 4;
        ucs4 = c & 0x03;
      }
      continue;
    }
    /* loop to split ucs4 into two utf16 chars if necessary */
    utf8total = 0;
    do
    {
      if ( ucs4 >= UTF16BASE ) {
        ucs4 -= UTF16BASE;
        bitbuf =
          ( bitbuf << 16 ) | ( ( ucs4 >> UTF16SHIFT ) + UTF16HIGHSTART );
        ucs4 = ( ucs4 & UTF16MASK ) + UTF16LOSTART;
        utf16flag = 1;
      } else {
        bitbuf = ( bitbuf << 16 ) | ucs4;
        utf16flag = 0;
      }
      bitstogo += 16;
      /* spew out base64 */
      while ( bitstogo >= 6 ) {
        bitstogo -= 6;
        dst +=
          base64chars[( bitstogo ? ( bitbuf >> bitstogo ) : bitbuf ) & 0x3F];
      }
    }
    while ( utf16flag );
  }
  /* if in UTF-7 mode, finish in ASCII */
  if ( utf7mode ) {
    if ( bitstogo ) {
      dst += base64chars[( bitbuf << ( 6 - bitstogo ) ) & 0x3F];
    }
    dst += '-';
  }
  return quoteIMAP( dst );
}
