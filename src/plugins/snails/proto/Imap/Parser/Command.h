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
#ifndef IMAP_COMMAND_H
#define IMAP_COMMAND_H

#include <QDateTime>
#include <QList>
#include <QTextStream>

/** @short Namespace for IMAP interaction */
namespace Imap {

// Forward required for friend declaration
class Parser;
class CommandResult;

    QTextStream& operator<<( QTextStream& stream, const CommandResult& r );

/** @short Namespace holding all supported IMAP commands and various helpers */
namespace Commands {

    /** Enumeration that specifies required method of transmission of this string */
    enum TokenType {
        ATOM /**< Don't use any extra encoding, just send it directly, perhaps because it's already encoded */,
        QUOTED_STRING /**< Transmit using double-quotes */,
        LITERAL /**< Don't bother with checking this data, always use literal form */,
        IDLE, /**< Special case: IDLE command */
        IDLE_DONE, /**< Special case: the DONE for finalizing the IDLE command */
        STARTTLS /**< Special case: STARTTLS */
    };

    /** @short Checks if we can use a quoted-string form for transmitting this string.
     *
     * We have to use literals for transmitting strings that are either too
     * long (as that'd cause problems with servers using too small line buffers),
     * contains CR, LF, zero byte or any characters outside of 7-bit ASCII range.
     */
    TokenType howToTransmit( const QString& str );

    /** @short A part of the actual command.
     *
     * This is used by Parser to decide whether
     * to send the string as-is, to quote them or use a literal form for them.
     */
    class PartOfCommand {
        TokenType _kind; /**< What encoding to use for this item */
        QString _text; /**< Actual text to send */
        bool _numberSent;

        friend QTextStream& operator<<( QTextStream& stream, const PartOfCommand& c );
        friend class ::Imap::Parser;

    public:
        /** Default constructor */
        PartOfCommand( const TokenType kind, const QString& text): _kind(kind), _text(text), _numberSent(false) {}
        /** Constructor that guesses correct type for passed string */
        PartOfCommand( const QString& text): _kind( howToTransmit(text) ), _text(text), _numberSent(false) {}
    };

    /** @short Abstract class for specifying what command to execute */
    class Command {
        friend QTextStream& operator<<( QTextStream& stream, const Command& c );
        friend class ::Imap::Parser;
        QList<PartOfCommand> _cmds;
        int _currentPart;
    public:
        Command& operator<<( const PartOfCommand& part ) { _cmds << part; return *this; }
        Command& operator<<( const QString& text ) { _cmds << PartOfCommand( text ); return *this; }
        Command(): _currentPart(0) {}
        Command( const QString& name ): _currentPart(0) { _cmds << PartOfCommand( ATOM, name ); }
        void addTag( const QString& tag ) { _cmds.insert( 0, PartOfCommand( ATOM, tag ) ); }
    };

    /** @short Used for dumping a command to debug stream */
    QTextStream& operator<<( QTextStream& stream, const Command& cmd );

}
}
#endif /* IMAP_COMMAND_H */
