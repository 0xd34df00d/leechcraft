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
#ifndef IMAP_RESPONSE_H
#define IMAP_RESPONSE_H

#include <QMap>
#include <QPair>
#include <QSharedPointer>
#include <QStringList>
#include <QTextStream>
#include <QVariantList>
#include <QVector>
#include "Command.h"
#include "../Exceptions.h"
#include "Data.h"
#include "ThreadingNode.h"

/**
 * @file
 * @short Various data structures related to IMAP responses
 *
 * @author Jan Kundrát <jkt@gentoo.org>
 */

/** @short Namespace for IMAP interaction */
namespace Imap {

namespace Mailbox {
    class Model;
    class ImapTask;
}

class Parser;

/** @short IMAP server responses
 *
 * @ref AbstractResponse is an abstact parent of all classes. Each response
 * that might be received from the server is a child of this one.
 * */
namespace Responses {

    /** @short Result of a command */
    enum Kind {
        OK /**< @short OK */,
        NO /**< @short NO */,
        BAD /**< @short BAD */,
        BYE,
        PREAUTH,
        EXPUNGE,
        FETCH,
        EXISTS,
        RECENT,
        CAPABILITY,
        LIST,
        LSUB,
        FLAGS,
        SEARCH,
        STATUS,
        NAMESPACE,
        SORT,
        THREAD
    }; // aren't those comments just sexy? :)

    /** @short Response Code */
    enum Code {
        NONE /**< @short No response code specified */,
        ATOM /**< @short Not recognized */,
        ALERT /**< @short ALERT */,
        BADCHARSET /**< @short BADCHARSET */,
        /** @short CAPABILITY.
         *
         * Yeah, it's different than the RFC3501 name for it.
         * Responses::Kind already defines a CAPABILITY and we aren't using
         * C++0x yet.
         *
         * */
        CAPABILITIES,
        PARSE /**< @short PARSE */,
        PERMANENTFLAGS /**< @short PERMANENTFLAGS */,
        READ_ONLY /**< @short READ-ONLY */,
        READ_WRITE /**< @short READ-WRITE */,
        TRYCREATE /**< @short TRYCREATE */,
        UIDNEXT /**< @short UIDNEXT */,
        UIDVALIDITY /**< @short UIDVALIDITY */,
        UNSEEN /**< @short UNSEEN */,

        // obsolete from RFC 2060
        NEWNAME /**< Obsolete NEWNAME from RFC 2060 */,

        // RFC 2221
        REFERRAL /**< REFERRAL from RFC 2221 */,

        // RFC 3516
        UNKNOWN_CTE /**< UNKNOWN-CTE from RFC 3516 */,

        // RFC 4315
        UIDNOTSTICKY /**< UIDNOTSTICKY from RFC 4315 */,
        APPENDUID /**< APPENDUID from RFC 4315 */,
        COPYUID /**< COPYUID from RFC 4315 */,

        // RFC 4467
        URLMECH /**< URLMECH from RFC 4467 */,

        // RFC 4469
        TOOBIG /**< RFC 4469's TOOBIG */,
        BADURL /**< BADURL from RFC 4469 */,

        // RFC 4551
        HIGHESTMODSEQ /**< HIGHESTMODSEQ from RFC 4451 */,
        NOMODSEQ /**< NOMODSEQ from RFC 4551 */,
        MODIFIED /**< MODIFIED from RFC 4551 */,

        // RFC 4978
        COMPRESSIONACTIVE /**< COMPRESSIONACTIVE from RFC 4978 */,

        // RFC 5162
        CLOSED /**< CLOSED from the QRESYNC RFC 5162 */,

        // RFC 5182
        NOTSAVED /**< NOTSAVED from RFC 5182 */,

        // RFC 5255
        BADCOMPARATOR /**< BADCOMPARATOR from RFC 5255 */,

        // RFC 5257
        ANNOTATE /**< ANNOTATE from RFC 5257 */,
        ANNOTATIONS /**< ANNOTATIONS from RFC 5257 */,

        // RFC 5259
        TEMPFAIL /**< TEMPFAIL from RFC 5259 */,
        MAXCONVERTMESSAGES /**< MAXCONVERTMESSAGES from RFC 5259 */,
        MAXCONVERTPARTS /**< MAXCONVERTPARTS from RFC 5259 */,

        // RFC 5267
        NOUPDATE /**< NOUPDATE from RFC 5267 */,

        // RFC 5464
        METADATA /**< METADATA from RFC 5464 */,

        // RFC 5465
        NOTIFICATIONOVERFLOW /**< NOTIFICATIONOVERFLOW from RFC 5465 */,
        BADEVENT /**< BADEVENT from RFC 5465 */,

        // RFC 5466
        UNDEFINED_FILTER /**< UNDEFINED-FILTER from RFC 5466 */,

        // RFC5530
        UNAVAILABLE /**< UNAVAILABLE from RFC 5530 */,
        AUTHENTICATIONFAILED /**< AUTHENTICATIONFAILED from RFC 5530 */,
        AUTHORIZATIONFAILED /**< AUTHORIZATIONFAILED from RFC 5530 */,
        EXPIRED /**< EXPIRED from RFC 5530 */,
        PRIVACYREQUIRED /**< PRIVACYREQUIRED from RFC 5530 */,
        CONTACTADMIN /**< CONTACTADMIN from RFC 5530 */,
        NOPERM /**< NOPERM from RFC 5530 */,
        INUSE /**< INUSE from RFC 5530 */,
        EXPUNGEISSUED /**< EXPUNGEISSUED from RFC 5530 */,
        CORRUPTION /**< CORRUPTION from RFC 5530 */,
        SERVERBUG /**< SERVERBUG from RFC 5530 */,
        CLIENTBUG /**< CLIENTBUG from RFC 5530 */,
        CANNOT /**< CANNOT from RFC 5530 */,
        LIMIT /**< LIMIT from RFC 5530 */,
        OVERQUOTA /**< OVERQUOTA from RFC 5530 */,
        ALREADYEXISTS /**< ALREADYEXISTS from RFC 5530 */,
        NONEXISTENT /**< NONEXISTENT from RFC 5530 */

    }; // luvly comments, huh? :)

    /** @short Parent class for all server responses */
    class AbstractResponse {
    public:
        /** @short Kind of reponse */
        Kind kind;
        AbstractResponse(): kind(BAD) {};
        AbstractResponse( const Kind _kind ): kind(_kind) {};
        virtual ~AbstractResponse();
        /** @short Helper for operator<<() */
        virtual QTextStream& dump( QTextStream& ) const = 0;
        /** @short Helper for operator==() */
        virtual bool eq( const AbstractResponse& other ) const = 0;
        /** @short Helper for Imap::Mailbox::MailboxModel to prevent ugly
         * dynamic_cast<>s */
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const = 0;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const = 0;
    };

    /** @short Structure storing OK/NO/BAD/PREAUTH/BYE responses */
    class State : public AbstractResponse {
    public:
        /** @short Tag name or QString::null if untagged */
        QString tag;

        /** @short Kind of response
         *
         * A tagged status response might be either OK, NO or BAD.
         * Untagged status response might be either te same as tagged or BYE or
         * PREAUTH.
         * */
        Kind kind;

        /** @short Textual information embedded in the response
         *
         * While this information might be handy for correct understanding of
         * what happens at ther server, its value is not standardized so the
         * meaning is usually either duplicate to what's already said elsewhere
         * or only a hint to the user. Nevertheless, we decode and store it.
         * */
        QString message;

        /** @short Kind of optional Response Code
         *
         * For each supported value, type of ResponseCodeData stored in the
         * respCodeData is defined as follows:
         *
         *  ALERT, PARSE, READ_ONLY, READ_WRITE, TRYCREATE:
         *      Nothing else should be included, ie. void
         *
         *  UIDNEXT, UIDVALIDITY, UNSEEN:
         *      Only number, ie. unsigned int
         *
         *  BADCHARSET, PERMANENTFLAGS:
         *      List of strings, ie. QStringList
         *
         *  default:
         *      Any data, ie. QString
         * */
        Code respCode;

        /** @short Response Code Data
         *
         * Format is explained in the respCode documentation.
         * We have to use pointer indirection because virtual methods wouldn't
         * work otherwise.
         * */
        QSharedPointer<AbstractData> respCodeData;

        /** @short Default constructor
         *
         * No error checking takes place, we assume _respCodeData's type
         * actually corresponds to all invariants we declare as per respCode's
         * documentation.
         * */
        State( const QString& _tag, const Kind _kind, const QString& _message,
                const Code _respCode,
                const QSharedPointer<AbstractData> _respCodeData ):
            tag(_tag), kind(_kind), message(_message), respCode(_respCode),
            respCodeData(_respCodeData) {};

        /** @short "Smart" constructor that parses a response out of a QByteArray */
        State( const QString& _tag, const Kind _kind, const QByteArray& line, int& start );

        /** @short Default destructor that makes containers and QtTest happy */
        State(): respCode(NONE) {};

        /** @short helper for operator<<( QTextStream& ) */
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short Structure storing a CAPABILITY untagged response */
    class Capability : public AbstractResponse {
    public:
        /** @short List of capabilities */
        QStringList capabilities;
        Capability( const QStringList& _caps ) : AbstractResponse(CAPABILITY), capabilities(_caps) {};
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short Structure for EXISTS/EXPUNGE/RECENT responses */
    class NumberResponse : public AbstractResponse {
    public:
        /** @short Number that we're storing */
        uint number;
        NumberResponse( const Kind _kind, const uint _num ) throw( UnexpectedHere );
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short Structure storing a LIST untagged response */
    class List : public AbstractResponse {
    public:
        /** @short LIST or LSUB */
        Kind kind;
        /** @short Flags for this particular mailbox */
        QStringList flags;
        /** @short Hierarchy separator
         *
         * QString::null in case original response containded NIL
         * */
        QString separator;
        /** @short Mailbox name */
        QString mailbox;

        /** @short Parse line and construct List object from it */
        List( const Kind _kind, const QByteArray& line, int& start );
        List( const Kind _kind, const QStringList& _flags, const QString& _separator, const QString& _mailbox ):
            AbstractResponse(LIST), kind(_kind), flags(_flags), separator(_separator), mailbox(_mailbox) {};
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    struct NamespaceData {
        QString prefix;
        QString separator;
        NamespaceData( const QString& _prefix, const QString& _separator ): prefix(_prefix), separator(_separator) {};
        bool operator==( const NamespaceData& other ) const;
        bool operator!=( const NamespaceData& other ) const;
        static QList<NamespaceData> listFromLine( const QByteArray& line, int& start );
    };

    /** @short Structure storing a NAMESPACE untagged response */
    class Namespace : public AbstractResponse {
    public:
        QList<NamespaceData> personal, users, other;
        /** @short Parse line and construct List object from it */
        Namespace( const QByteArray& line, int& start );
        Namespace( const QList<NamespaceData>& _personal, const QList<NamespaceData>& _users,
                const QList<NamespaceData>& _other ):
            personal(_personal), users(_users), other(_other) {};
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };


    /** @short Structure storing a FLAGS untagged response */
    class Flags : public AbstractResponse {
    public:
        /** @short List of flags */
        QStringList flags;
        Flags( const QStringList& _flags ) : AbstractResponse(FLAGS), flags(_flags) {};
        Flags( const QByteArray& line, int& start );
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short Structure storing a SEARCH untagged response */
    class Search : public AbstractResponse {
    public:
        /** @short List of matching messages */
        QList<uint> items;
        Search( const QByteArray& line, int& start );
        Search( const QList<uint>& _items ) : AbstractResponse(SEARCH), items(_items) {};
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short Structure storing a STATUS untagged response */
    class Status : public AbstractResponse {
    public:
        /** @short Indentifies type of status data */
        enum StateKind {
            MESSAGES,
            RECENT,
            UIDNEXT,
            UIDVALIDITY,
            UNSEEN
        };

        typedef QMap<StateKind,uint> stateDataType;

        /** @short Mailbox name */
        QString mailbox;
        /** @short Associative array of states */
        stateDataType states;

        Status( const QString& _mailbox, const stateDataType& _states ) :
            AbstractResponse(STATUS), mailbox(_mailbox), states(_states) {};
        Status( const QByteArray& line, int& start );
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        static StateKind stateKindFromStr( QString s );
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short FETCH response */
    class Fetch : public AbstractResponse {
    public:
        typedef QMap<QString,QSharedPointer<AbstractData> > dataType;

        /** @short Sequence number of message that we're working with */
        uint number;

        /** @short Fetched items */
        dataType data;

        Fetch( const uint _number, const QByteArray& line, int& start );
        Fetch( const uint _number, const dataType& _data );
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    private:
        static QDateTime dateify( QByteArray str, const QByteArray& line, const int start );
    };

    /** @short Structure storing a SORT untagged response */
    class Sort : public AbstractResponse {
    public:
        /** @short List of sequence/UID numbers as returned by the server */
        QList<uint> numbers;
        Sort( const QByteArray& line, int& start );
        Sort( const QList<uint>& items ): AbstractResponse(SORT), numbers(items) {}
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    };

    /** @short Structure storing a THREAD untagged response */
    class Thread : public AbstractResponse {
    public:
        /** @short List of "top-level" messages */
        QVector<ThreadingNode> rootItems;
        Thread( const QByteArray& line, int& start );
        Thread( const QVector<ThreadingNode>& items ): AbstractResponse(THREAD), rootItems(items) {}
        virtual QTextStream& dump( QTextStream& s ) const;
        virtual bool eq( const AbstractResponse& other ) const;
        virtual void plug( Imap::Parser* parser, Imap::Mailbox::Model* model ) const;
        virtual bool plug( Imap::Mailbox::ImapTask* task ) const;
    private:
        void insertHere( ThreadingNode* where, const QVariantList& what );
        static QString dumpHelper( const ThreadingNode& node );
    };


    QTextStream& operator<<( QTextStream& stream, const Code& r );
    QTextStream& operator<<( QTextStream& stream, const Kind& res );
    QTextStream& operator<<( QTextStream& stream, const Status::StateKind& kind );
    QTextStream& operator<<( QTextStream& stream, const AbstractResponse& res );
    QTextStream& operator<<( QTextStream& stream, const NamespaceData& res );

    inline bool operator==( const AbstractResponse& first, const AbstractResponse& other ) {
        return first.eq( other );
    }

    inline bool operator!=( const AbstractResponse& first, const AbstractResponse& other ) {
        return !first.eq( other );
    }

    /** @short Build Responses::Kind from textual value */
    Kind kindFromString( QByteArray str ) throw( UnrecognizedResponseKind );

}

}

#endif // IMAP_RESPONSE_H
