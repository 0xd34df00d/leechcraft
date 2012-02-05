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
#ifndef IMAP_DATA_H
#define IMAP_DATA_H

#include <QTextStream>

/** @short Namespace for IMAP interaction */
namespace Imap {

/** @short IMAP server responses */
namespace Responses {

    /** @short Parent of all "Response Code Data" classes
     *
     * More information available in AbstractData's documentation.
     * */
    class AbstractData {
    public:
        virtual ~AbstractData();
        virtual QTextStream& dump( QTextStream& ) const = 0;
        virtual bool eq( const AbstractData& other ) const = 0;
    };

    /** @short Storage for "Response Code Data"
     *
     * In IMAP, each status response might contain some additional information
     * called "Response Code" and associated data. These data come in several
     * shapes and this class servers as a storage for them, as a kind of
     * QVariant-like wrapper around real data.
     * */
    template<class T> class RespData : public AbstractData {
    public:
        T data;
        RespData( const T& _data ) : data(_data) {};
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractData& other ) const;
    };

    /** Explicit specialization for void as we can't define a void member of a
     * class */
    template<> class RespData<void> : public AbstractData {
    public:
        virtual QTextStream& dump( QTextStream& s ) const { return s; };
        virtual bool eq( const AbstractData& other ) const;
    };


    QTextStream& operator<<( QTextStream& stream, const AbstractData& resp );

    inline bool operator==( const AbstractData& first, const AbstractData& other ) {
        return first.eq( other );
    }

    inline bool operator!=( const AbstractData& first, const AbstractData& other ) {
        return !first.eq( other );
    }

}

}

#endif /* IMAP_DATA_H */
