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
#ifndef IMAP_EXCEPTIONS_H
#define IMAP_EXCEPTIONS_H
#include <exception>
#include <string>
#include <QByteArray>

/**
 * @file
 * @short Common IMAP-related exceptions
 *
 * All IMAP-related exceptions inherit from Imap::Exception which inherits from
 * std::exception.
 *
 * @author Jan Kundrát <jkt@gentoo.org>
 */

/** @short Namespace for IMAP interaction */
namespace Imap {

namespace Responses {
    class AbstractResponse;
}

    /** @short General exception class */
    class ImapException : public std::exception {
    protected:
        /** The error message */
        std::string _msg;
        /** Line with data that caused this error */
        QByteArray _line;
        /** Offset in line for error source */
        int _offset;
        /** Class name of the exception */
        std::string _exceptionClass;
    public:
        ImapException(): _offset(-1), _exceptionClass("ImapException") {}
        ImapException( const std::string& msg ) : _msg(msg), _offset(-1), _exceptionClass("ImapException") {};
        ImapException( const std::string& msg, const QByteArray& line, const int offset ):
            _msg(msg), _line(line), _offset(offset), _exceptionClass("ImapException") {};
        virtual const char* what() const throw();
        virtual ~ImapException() throw() {};
        std::string msg() const { return _msg; }
        QByteArray line() const { return _line; }
        int offset() const { return _offset; }
        std::string exceptionClass() const { return _exceptionClass; }
    };

#define ECBODY(CLASSNAME, PARENT) class CLASSNAME: public PARENT { \
    public: CLASSNAME( const std::string& msg ): PARENT(msg ) { _exceptionClass = #CLASSNAME; }\
    CLASSNAME( const QByteArray& line, const int offset ): PARENT( #CLASSNAME, line, offset ) { _exceptionClass = #CLASSNAME; }\
    CLASSNAME( const std::string& msg, const QByteArray& line, const int offset ): PARENT( msg, line, offset ) { _exceptionClass = #CLASSNAME; }\
    };

    /** @short The STARTTLS command failed */
    ECBODY(StartTlsFailed, ImapException)

    /** @short A generic parser exception */
    ECBODY(ParserException, ImapException)

    /** @short Invalid argument was passed to some function */
    ECBODY(InvalidArgument, ParserException)

    /** @short Socket error */
    ECBODY(SocketException, ParserException)

    /** @short Waiting for something from the socket took too long */
    ECBODY(SocketTimeout, SocketException)

    /** @short General parse error */
    ECBODY(ParseError, ParserException)

    /** @short Parse error: unknown identifier */
    ECBODY(UnknownIdentifier, ParseError)

    /** @short Parse error: unrecognized kind of response */
    ECBODY(UnrecognizedResponseKind, UnknownIdentifier)

    /** @short Parse error: this is known, but not expected here */
    ECBODY(UnexpectedHere, ParseError)

    /** @short Parse error: No usable data */
    ECBODY(NoData, ParseError)

    /** @short Parse error: Too much data */
    ECBODY(TooMuchData, ParseError)

    /** @short Command Continuation Request received, but we have no idea how to handle it here */
    ECBODY(ContinuationRequest, ParserException)

    /** @short Invalid Response Code */
    ECBODY(InvalidResponseCode, ParseError)

#undef ECBODY

    /** @short Parent for all exceptions thrown by Imap::Mailbox-related classes */
    class MailboxException: public ImapException {
    public:
        MailboxException( const char* const msg, const Imap::Responses::AbstractResponse& response );
        MailboxException( const char* const msg );
        virtual const char* what() const throw () { return _msg.c_str(); };
        virtual ~MailboxException() throw () {};

    };

#define ECBODY(CLASSNAME, PARENT) class CLASSNAME: public PARENT { \
    public: CLASSNAME(const char* const msg, const Imap::Responses::AbstractResponse& response): PARENT(msg, response) { _exceptionClass=#CLASSNAME; } \
    CLASSNAME(const char* const msg): PARENT(msg) { _exceptionClass=#CLASSNAME; } \
    };

    /** @short Server sent us something that isn't expected right now */
    ECBODY(UnexpectedResponseReceived, MailboxException)

    /** @short Internal error in Imap::Mailbox code -- there must be bug in its code */
    ECBODY(CantHappen, MailboxException)

    /** @short Server sent us information about message we don't know */
    ECBODY(UnknownMessageIndex, MailboxException)

#undef ECBODY

}
#endif /* IMAP_EXCEPTIONS_H */
