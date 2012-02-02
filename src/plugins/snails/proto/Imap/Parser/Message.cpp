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

#include <typeinfo>

#include <QTextDocument>
#include <QUrl>
#include "Message.h"
#include "../Model/MailboxTree.h"
#include "../Encoders.h"

namespace Imap {
namespace Message {

QList<MailAddress> Envelope::getListOfAddresses( const QVariant& in, const QByteArray& line, const int start )
{
    if ( in.type() == QVariant::ByteArray ) {
        if ( ! in.toByteArray().isNull() )
            throw UnexpectedHere( "getListOfAddresses: byte array not null", line, start );
    } else if ( in.type() != QVariant::List ) {
        throw ParseError( "getListOfAddresses: not a list", line, start );
    }

    QVariantList list = in.toList();
    QList<MailAddress> res;
    for ( QVariantList::const_iterator it = list.begin(); it != list.end(); ++it ) {
        if ( it->type() != QVariant::List )
            throw UnexpectedHere( "getListOfAddresses: split item not a list", line, start ); // FIXME: wrong offset
        res.append( MailAddress( it->toList(), line, start ) );
    }
    return res;
}

MailAddress::MailAddress( const QVariantList& input, const QByteArray& line, const int start )
{
    // FIXME: all offsets are wrong here
    if ( input.size() != 4 )
        throw ParseError( "MailAddress: not four items", line, start );

    if ( input[0].type() != QVariant::ByteArray )
        throw UnexpectedHere( "MailAddress: item#1 not a QByteArray", line, start );
    if ( input[1].type() != QVariant::ByteArray )
        throw UnexpectedHere( "MailAddress: item#2 not a QByteArray", line, start );
    if ( input[2].type() != QVariant::ByteArray )
        throw UnexpectedHere( "MailAddress: item#3 not a QByteArray", line, start );
    if ( input[3].type() != QVariant::ByteArray )
        throw UnexpectedHere( "MailAddress: item#4 not a QByteArray", line, start );

    name = Imap::decodeRFC2047String( input[0].toByteArray() );
    adl = Imap::decodeRFC2047String( input[1].toByteArray() );
    mailbox = Imap::decodeRFC2047String( input[2].toByteArray() );
    host = Imap::decodeRFC2047String( input[3].toByteArray() );
}

QString MailAddress::prettyName( FormattingMode mode ) const
{
    if ( name.isEmpty() && mode == FORMAT_JUST_NAME )
        mode = FORMAT_READABLE;
    
    if ( mode == FORMAT_JUST_NAME ) {
        return name;
    } else {
        QString address = mailbox + QChar('@') + host;
        QString result;
        QString niceName;
        if ( name.isEmpty() ) {
            result = address;
            niceName = address;
        } else {
            result = name + QString::fromAscii( " <" ) + address + QChar( '>' );
            niceName = name;
        }
        if ( mode == FORMAT_READABLE ) {
            return result;
        } else {
            QUrl target;
            target.setScheme( QLatin1String("mailto") );
            target.setUserName( mailbox );
            target.setHost( host );
            target.addQueryItem( QLatin1String("X-Trojita-DisplayName"), niceName );
            return QString::fromAscii( "<a href=\"%1\">%2</a>" ).arg( Qt::escape( target.toString() ), Qt::escape( niceName ) );
        }
    }
}

QString MailAddress::prettyList( const QList<MailAddress>& list, FormattingMode mode )
{
    QStringList buf;
    for ( QList<MailAddress>::const_iterator it = list.begin(); it != list.end(); ++it )
        buf << it->prettyName( mode );
    return buf.join( QString::fromAscii(", ") );
}

QString MailAddress::prettyList( const QVariantList& list, FormattingMode mode )
{
    QStringList buf;
    for ( QVariantList::const_iterator it = list.begin(); it != list.end(); ++it ) {
        Q_ASSERT(it->type() == QVariant::StringList);
        QStringList item = it->toStringList();
        Q_ASSERT(item.size() == 4);
        MailAddress a( item[0], item[1], item[2], item[3] );
        buf << a.prettyName( mode );
    }
    return buf.join( QString::fromAscii(", ") );
}

Envelope Envelope::fromList( const QVariantList& items, const QByteArray& line, const int start )
{
    if ( items.size() != 10 )
        throw ParseError( "Envelope::fromList: size != 10", line, start ); // FIXME: wrong offset

    // date
    QDateTime date;
    if ( items[0].type() == QVariant::ByteArray ) {
        QByteArray dateStr = items[0].toByteArray();
        if ( ! dateStr.isEmpty() ) {
            try {
                date = LowLevelParser::parseRFC2822DateTime( dateStr );
            } catch ( ParseError& e ) {
                // FIXME: log this
                //throw ParseError( e.what(), line, start );
            }
        }
    }
    // Otherwise it's "invalid", null.

    QString subject = Imap::decodeRFC2047String( items[1].toByteArray() );

    QList<MailAddress> from, sender, replyTo, to, cc, bcc;
    from = Envelope::getListOfAddresses( items[2], line, start );
    sender = Envelope::getListOfAddresses( items[3], line, start );
    replyTo = Envelope::getListOfAddresses( items[4], line, start );
    to = Envelope::getListOfAddresses( items[5], line, start );
    cc = Envelope::getListOfAddresses( items[6], line, start );
    bcc = Envelope::getListOfAddresses( items[7], line, start );

    if ( items[8].type() != QVariant::ByteArray )
        throw UnexpectedHere( "Envelope::fromList: inReplyTo not a QByteArray", line, start );
    QByteArray inReplyTo = items[8].toByteArray();

    if ( items[9].type() != QVariant::ByteArray )
        throw UnexpectedHere( "Envelope::fromList: messageId not a QByteArray", line, start );
    QByteArray messageId = items[9].toByteArray();

    return Envelope( date, subject, from, sender, replyTo, to, cc, bcc, inReplyTo, messageId );
}

void Envelope::clear()
{
    date = QDateTime();
    subject.clear();
    from.clear();
    sender.clear();
    replyTo.clear();
    to.clear();
    cc.clear();
    bcc.clear();
    inReplyTo.clear();
    messageId.clear();
}

bool OneMessage::eq( const AbstractData& other ) const
{
    try {
        const OneMessage& o = dynamic_cast<const OneMessage&>( other );
        return o.mediaType == mediaType && mediaSubType == o.mediaSubType &&
            bodyFldParam == o.bodyFldParam && bodyFldId == o.bodyFldId &&
            bodyFldDesc == o.bodyFldDesc && bodyFldEnc == o.bodyFldEnc &&
            bodyFldOctets == o.bodyFldOctets && bodyFldMd5 == o.bodyFldMd5 &&
            bodyFldDsp == o.bodyFldDsp && bodyFldLang == o.bodyFldLang &&
            bodyFldLoc == o.bodyFldLoc && bodyExtension == o.bodyExtension;
    } catch ( std::bad_cast& ) {
        return false;
    }
}

bool TextMessage::eq( const AbstractData& other ) const
{
    try {
        const TextMessage& o = dynamic_cast<const TextMessage&>( other );
        return OneMessage::eq( o ) && bodyFldLines == o.bodyFldLines;
    } catch ( std::bad_cast& ) {
        return false;
    }
}

QTextStream& TextMessage::dump( QTextStream& s, const int indent ) const
{
    QByteArray i( indent + 1, ' ' );
    QByteArray lf( "\n" );

    return s << QByteArray( indent, ' ' ) << "TextMessage( " << mediaType << "/" << mediaSubType << lf <<
        i << "body-fld-param: " << bodyFldParam << lf <<
        i << "body-fld-id: " << bodyFldId << lf <<
        i << "body-fld-desc: " << bodyFldDesc << lf <<
        i << "body-fld-enc: " << bodyFldEnc << lf <<
        i << "body-fld-octets: " << bodyFldOctets << lf <<
        i << "bodyFldMd5: " << bodyFldMd5 << lf <<
        i << "body-fld-dsp: " << bodyFldDsp << lf <<
        i << "body-fld-lang: " << bodyFldLang << lf <<
        i << "body-fld-loc: " << bodyFldLoc << lf <<
        i << "body-extension is " << bodyExtension.typeName() << lf <<
        i << "body-fld-lines: " << bodyFldLines << lf <<
        QByteArray( indent, ' ' ) << ")";
    // FIXME: operator<< for QVariant...
}

bool MsgMessage::eq( const AbstractData& other ) const
{
    try {
        const MsgMessage& o = dynamic_cast<const MsgMessage&>( other );
        if ( o.body ) {
            if ( body ) {
                if ( *body != *o.body ) {
                    return false;
                }
            } else {
                return false;
            }
        } else if ( body ) {
            return false;
        }

        return OneMessage::eq( o ) && bodyFldLines == o.bodyFldLines &&
            envelope == o.envelope;

    } catch ( std::bad_cast& ) {
        return false;
    }
}

QTextStream& MsgMessage::dump( QTextStream& s, const int indent ) const
{
    QByteArray i( indent + 1, ' ' );
    QByteArray lf( "\n" );

    s << QByteArray( indent, ' ' ) << "MsgMessage(" << lf;
    envelope.dump( s, indent + 1 );
    s <<
        i << "body-fld-lines " << bodyFldLines << lf <<
        i << "body:" << lf;

    s <<
    i << "body-fld-param: " << bodyFldParam << lf <<
    i << "body-fld-id: " << bodyFldId << lf <<
    i << "body-fld-desc: " << bodyFldDesc << lf <<
    i << "body-fld-enc: " << bodyFldEnc << lf <<
    i << "body-fld-octets: " << bodyFldOctets << lf <<
    i << "bodyFldMd5: " << bodyFldMd5 << lf <<
    i << "body-fld-dsp: " << bodyFldDsp << lf <<
    i << "body-fld-lang: " << bodyFldLang << lf <<
    i << "body-fld-loc: " << bodyFldLoc << lf <<
    i << "body-extension is " << bodyExtension.typeName() << lf;

    if ( body )
        body->dump( s, indent + 2 );
    else
        s << i << " (null)";
    return s << lf << QByteArray( indent, ' ' ) << ")";
}

QTextStream& BasicMessage::dump( QTextStream& s, const int indent ) const
{
    QByteArray i( indent + 1, ' ' );
    QByteArray lf( "\n" );

    return s << QByteArray( indent, ' ' ) << "BasicMessage( " << mediaType << "/" << mediaSubType << lf <<
        i << "body-fld-param: " << bodyFldParam << lf <<
        i << "body-fld-id: " << bodyFldId << lf <<
        i << "body-fld-desc: " << bodyFldDesc << lf <<
        i << "body-fld-enc: " << bodyFldEnc << lf <<
        i << "body-fld-octets: " << bodyFldOctets << lf <<
        i << "bodyFldMd5: " << bodyFldMd5 << lf <<
        i << "body-fld-dsp: " << bodyFldDsp << lf <<
        i << "body-fld-lang: " << bodyFldLang << lf <<
        i << "body-fld-loc: " << bodyFldLoc << lf <<
        i << "body-extension is " << bodyExtension.typeName() << lf <<
        QByteArray( indent, ' ' ) << ")";
    // FIXME: operator<< for QVariant...
}

bool MultiMessage::eq( const AbstractData& other ) const
{
    try {
        const MultiMessage& o = dynamic_cast<const MultiMessage&>( other );

        if ( bodies.count() != o.bodies.count() ) {
            return false;
        }

        for ( int i = 0; i < bodies.count(); ++i ) {
            if ( bodies[i] ) {
                if ( o.bodies[i] ) {
                    if ( *bodies[i] != *o.bodies[i] ) {
                        return false;
                    }
                } else {
                    return false;
                }
            } else if ( ! o.bodies[i] ) {
                return false;
            }
        }

        return mediaSubType == o.mediaSubType && bodyFldParam == o.bodyFldParam &&
            bodyFldDsp == o.bodyFldDsp && bodyFldLang == o.bodyFldLang &&
            bodyFldLoc == o.bodyFldLoc && bodyExtension == o.bodyExtension;

    } catch ( std::bad_cast& ) {
        return false;
    }
}

QTextStream& MultiMessage::dump( QTextStream& s, const int indent ) const
{
    QByteArray i( indent + 1, ' ' );
    QByteArray lf( "\n" );

    s << QByteArray( indent, ' ' ) << "MultiMessage( multipart/" << mediaSubType << lf <<
        i << "body-fld-param " << bodyFldParam << lf <<
        i << "body-fld-dsp " << bodyFldDsp << lf <<
        i << "body-fld-lang " << bodyFldLang << lf <<
        i << "body-fld-loc " << bodyFldLoc << lf <<
        i << "bodyExtension is " << bodyExtension.typeName() << lf <<
        i << "bodies: [ " << lf;

    for ( QList<QSharedPointer<AbstractMessage> >::const_iterator it = bodies.begin(); it != bodies.end(); ++it )
        if ( *it ) {
            (**it).dump( s, indent + 2 );
            s << lf;
        } else
            s << i << " (null)" << lf;

    return s << QByteArray( indent, ' ' ) << "] )";
}

AbstractMessage::bodyFldParam_t AbstractMessage::makeBodyFldParam( const QVariant& input, const QByteArray& line, const int start )
{
    bodyFldParam_t map;
    if ( input.type() != QVariant::List ) {
        if ( input.type() == QVariant::ByteArray && input.toByteArray().isNull() )
            return map;
        throw UnexpectedHere( "body-fld-param: not a list / nil", line, start );
    }
    QVariantList list = input.toList();
    if ( list.size() % 2 )
        throw UnexpectedHere( "body-fld-param: wrong number of entries", line, start );
    for ( int j = 0; j < list.size(); j += 2 )
        if ( list[j].type() != QVariant::ByteArray || list[j+1].type() != QVariant::ByteArray )
            throw UnexpectedHere( "body-fld-param: string not found", line, start );
        else
            map[ list[j].toByteArray().toUpper() ] = list[j+1].toByteArray();
    return map;
}

AbstractMessage::bodyFldDsp_t AbstractMessage::makeBodyFldDsp( const QVariant& input, const QByteArray& line, const int start )
{
    bodyFldDsp_t res;

    if ( input.type() != QVariant::List ) {
        if ( input.type() == QVariant::ByteArray && input.toByteArray().isNull() )
            return res;
        throw UnexpectedHere( "body-fld-dsp: not a list / nil", line, start );
    }
    QVariantList list = input.toList();
    if ( list.size() != 2 )
        throw ParseError( "body-fld-dsp: wrong number of entries in the list", line, start );
    if ( list[0].type() != QVariant::ByteArray )
        throw UnexpectedHere( "body-fld-dsp: first item is not a string", line, start );
    res.first = list[0].toByteArray();
    res.second = makeBodyFldParam( list[1], line, start );
    return res;
}

QList<QByteArray> AbstractMessage::makeBodyFldLang( const QVariant& input, const QByteArray& line, const int start )
{
    QList<QByteArray> res;
    if ( input.type() == QVariant::ByteArray ) {
        if ( input.toByteArray().isNull() ) // handle NIL
            return res;
        res << input.toByteArray();
    } else if ( input.type() == QVariant::List ) {
        QVariantList list = input.toList();
        for ( QVariantList::const_iterator it = list.begin(); it != list.end(); ++it )
            if ( it->type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-lang has wrong structure", line, start );
            else
                res << it->toByteArray();
    } else
        throw UnexpectedHere( "body-fld-lang not found", line, start );
    return res;
}

uint AbstractMessage::extractUInt( const QVariant& var, const QByteArray& line, const int start )
{
    if ( var.type() == QVariant::UInt )
        return var.toUInt();
    if ( var.type() == QVariant::ByteArray && var.toByteArray() == QByteArray("-1") ) {
        qDebug() << "Parser warning: -1 is not an unsigned int";
        return 0;
    }
    throw UnexpectedHere( "extractUInt: weird data type", line, start );
}


QSharedPointer<AbstractMessage> AbstractMessage::fromList( const QVariantList& items, const QByteArray& line, const int start )
{
    if ( items.size() < 2 )
        throw NoData( "AbstractMessage::fromList: no data", line, start );

    if ( items[0].type() == QVariant::ByteArray ) {
        // it's a single-part message, hurray

        int i = 0;
        QString mediaType = items[i].toString().toLower();
        ++i;
        QString mediaSubType = items[i].toString().toLower();
        ++i;

        if ( items.size() < 7 ) {
            qDebug() << "AbstractMessage::fromList(): body-type-basic(?): yuck, too few items, using what we've got";
        }

        bodyFldParam_t bodyFldParam;
        if ( i < items.size() ) {
            bodyFldParam = makeBodyFldParam( items[i], line, start );
            ++i;
        }

        QByteArray bodyFldId;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-id not recognized as a ByteArray", line, start );
            bodyFldId = items[i].toByteArray();
            ++i;
        }

        QByteArray bodyFldDesc;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-desc not recognized as a ByteArray", line, start );
            bodyFldDesc = items[i].toByteArray();
            ++i;
        }

        QByteArray bodyFldEnc;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-enc not recognized as a ByteArray", line, start );
            bodyFldEnc = items[i].toByteArray();
            ++i;
        }

        uint bodyFldOctets = 0;
        if ( i < items.size() ) {
            bodyFldOctets = extractUInt( items[i], line, start );
            ++i;
        }

        uint bodyFldLines = 0;
        Envelope envelope;
        QSharedPointer<AbstractMessage> body;

        enum { MESSAGE, TEXT, BASIC} kind;

        if ( mediaType == "message" && mediaSubType == "rfc822" ) {
            // extract envelope, body, body-fld-lines

            if ( items.size() < 10 )
                throw NoData( "too few fields for a Message-message", line, start );

            kind = MESSAGE;
            if ( items[i].type() == QVariant::ByteArray && items[i].toByteArray().isEmpty() ) {
                // ENVELOPE is NIL, this shouldn't really happen
                qDebug() << "AbstractMessage::fromList(): message/rfc822: yuck, got NIL for envelope";
            } else if ( items[i].type() != QVariant::List ) {
                throw UnexpectedHere( "message/rfc822: envelope not a list", line, start );
            } else {
                envelope = Envelope::fromList( items[i].toList(), line, start );
            }
            ++i;

            if ( items[i].type() != QVariant::List )
                throw UnexpectedHere( "message/rfc822: body not recognized as a list", line, start );
            body = AbstractMessage::fromList( items[i].toList(), line, start );
            ++i;

            try {
                bodyFldLines = extractUInt( items[i], line, start );
            } catch ( const UnexpectedHere & e ) {
                qDebug() << "AbstractMessage::fromList(): message/rfc822: yuck, invalid body-fld-lines";
            }
            ++i;

        } else if ( mediaType == "text" ) {
            if ( i < items.size() ) {
                // extract body-fld-lines
                kind = TEXT;
                bodyFldLines = extractUInt( items[i], line, start );
                ++i;
            }
        } else {
            // don't extract anything as we're done here
            kind = BASIC;
        }

        // extract body-ext-1part

        // body-fld-md5
        QByteArray bodyFldMd5;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-md5 not a ByteArray", line, start );
            bodyFldMd5 = items[i].toByteArray();
            ++i;
        }

        // body-fld-dsp
        bodyFldDsp_t bodyFldDsp;
        if ( i < items.size() ) {
            bodyFldDsp = makeBodyFldDsp( items[i], line, start );
            ++i;
        }

        // body-fld-lang
        QList<QByteArray> bodyFldLang;
        if ( i < items.size() ) {
            bodyFldLang = makeBodyFldLang( items[i], line, start );
            ++i;
        }

        // body-fld-loc
        QByteArray bodyFldLoc;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-loc not found", line, start );
            bodyFldLoc = items[i].toByteArray();
            ++i;
        }

        // body-extension
        QVariant bodyExtension;
        if ( i < items.size() ) {
            if ( i == items.size() - 1 ) {
                bodyExtension = items[i];
                ++i;
            } else {
                QVariantList list;
                for ( ; i < items.size(); ++i )
                    list << items[i];
                bodyExtension = list;
            }
        }

        switch ( kind ) {
            case MESSAGE:
                return QSharedPointer<AbstractMessage>(
                    new MsgMessage( mediaType, mediaSubType, bodyFldParam,
                        bodyFldId, bodyFldDesc, bodyFldEnc, bodyFldOctets,
                        bodyFldMd5, bodyFldDsp, bodyFldLang, bodyFldLoc,
                        bodyExtension, envelope, body, bodyFldLines )
                    );
            case TEXT:
                return QSharedPointer<AbstractMessage>(
                    new TextMessage( mediaType, mediaSubType, bodyFldParam,
                        bodyFldId, bodyFldDesc, bodyFldEnc, bodyFldOctets,
                        bodyFldMd5, bodyFldDsp, bodyFldLang, bodyFldLoc,
                        bodyExtension, bodyFldLines )
                    );
            case BASIC:
            default:
                return QSharedPointer<AbstractMessage>(
                    new BasicMessage( mediaType, mediaSubType, bodyFldParam,
                        bodyFldId, bodyFldDesc, bodyFldEnc, bodyFldOctets,
                        bodyFldMd5, bodyFldDsp, bodyFldLang, bodyFldLoc,
                        bodyExtension )
                    );
        }

    } else if ( items[0].type() == QVariant::List ) {

        if ( items.size() < 2 )
            throw ParseError( "body-type-mpart: structure should be \"body* string\"", line, start );

        int i = 0;

        QList<QSharedPointer<AbstractMessage> > bodies;
        while ( items[i].type() == QVariant::List) {
            bodies << fromList( items[i].toList(), line, start );
            ++i;
        }

        if ( items[i].type() != QVariant::ByteArray )
            throw UnexpectedHere( "body-type-mpart: media-subtype not recognized", line, start );
        QString mediaSubType = items[i].toString();
        ++i;

        // body-ext-mpart

        // body-fld-param
        bodyFldParam_t bodyFldParam;
        if ( i < items.size() ) {
            bodyFldParam = makeBodyFldParam( items[i], line, start );
            ++i;
        }

        // body-fld-dsp
        bodyFldDsp_t bodyFldDsp;
        if ( i < items.size() ) {
            bodyFldDsp = makeBodyFldDsp( items[i], line, start );
            ++i;
        }

        // body-fld-lang
        QList<QByteArray> bodyFldLang;
        if ( i < items.size() ) {
            bodyFldLang = makeBodyFldLang( items[i], line, start );
            ++i;
        }

        // body-fld-loc
        QByteArray bodyFldLoc;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( "body-fld-loc not found", line, start );
            bodyFldLoc = items[i].toByteArray();
            ++i;
        }

        // body-extension
        QVariant bodyExtension;
        if ( i < items.size() ) {
            if ( i == items.size() - 1 ) {
                bodyExtension = items[i];
                ++i;
            } else {
                QVariantList list;
                for ( ; i < items.size(); ++i )
                    list << items[i];
                bodyExtension = list;
            }
        }

        return QSharedPointer<AbstractMessage>(
                new MultiMessage( bodies, mediaSubType, bodyFldParam,
                    bodyFldDsp, bodyFldLang, bodyFldLoc, bodyExtension ) );
    } else {
        throw UnexpectedHere( "AbstractMessage::fromList: invalid data type of first item", line, start );
    }
}

QTextStream& operator<<( QTextStream& stream, const MailAddress& address )
{
    stream << '"' << address.name << "\" <";
    if ( !address.host.isNull() )
        stream << address.mailbox << '@' << address.host;
    else
        stream << address.mailbox;
    stream << '>';
    return stream;
}

void dumpListOfAddresses( QTextStream& stream, const QList<MailAddress>& list, const int indent )
{
    QByteArray lf( "\n" );
    switch ( list.size() ) {
        case 0:
            stream << "[ ]" << lf;
            break;
        case 1:
            stream << "[ " << list.front() << " ]" << lf;
            break;
        default:
            {
                QByteArray i( indent + 1, ' ' );
                stream << "[" << lf;
                for ( QList<MailAddress>::const_iterator it = list.begin(); it != list.end(); ++it )
                    stream << i << *it << lf;
                stream << QByteArray( indent, ' ' ) << "]" << lf;
            }
    }
}

QTextStream& Envelope::dump( QTextStream& stream, const int indent ) const
{
    QByteArray i( indent + 1, ' ' );
    QByteArray lf( "\n" );
    stream << QByteArray( indent, ' ' ) << "Envelope(" << lf <<
        i << "Date: " << date.toString() << lf <<
        i << "Subject: " << subject << lf;
    stream << i << "From: "; dumpListOfAddresses( stream, from, indent + 1 );
    stream << i << "Sender: "; dumpListOfAddresses( stream, sender, indent + 1 );
    stream << i << "Reply-To: "; dumpListOfAddresses( stream, replyTo, indent + 1 );
    stream << i << "To: "; dumpListOfAddresses( stream, to, indent + 1 );
    stream << i << "Cc: "; dumpListOfAddresses( stream, cc, indent + 1 );
    stream << i << "Bcc: "; dumpListOfAddresses( stream, bcc, indent + 1 );
    stream <<
        i << "In-Reply-To: " << inReplyTo << lf <<
        i << "Message-Id: " << messageId << lf;
    return stream << QByteArray( indent, ' ' ) << ")" << lf;
}

QTextStream& operator<<( QTextStream& stream, const Envelope& e )
{
    return e.dump( stream, 0 );
}

QTextStream& operator<<( QTextStream& stream, const AbstractMessage::bodyFldParam_t& p )
{
    stream << "bodyFldParam[ ";
    bool first = true;
    for ( AbstractMessage::bodyFldParam_t::const_iterator it = p.begin(); it != p.end(); ++it, first = false )
        stream << ( first ? "" : ", " ) << it.key() << ": " << it.value();
    return stream << "]";
}

QTextStream& operator<<( QTextStream& stream, const AbstractMessage::bodyFldDsp_t& p )
{
    return stream << "bodyFldDsp( " << p.first << ", " << p.second << ")";
}

QTextStream& operator<<( QTextStream& stream, const QList<QByteArray>& list )
{
    stream << "( ";
    bool first = true;
    for ( QList<QByteArray>::const_iterator it = list.begin(); it != list.end(); ++it, first = false )
        stream << ( first ? "" : ", " ) << *it;
    return stream << " )";
}

bool operator==( const Envelope& a, const Envelope& b )
{
    return a.date == b.date && a.date == b.date && a.subject == b.subject &&
        a.from == b.from && a.sender == b.sender && a.replyTo == b.replyTo &&
        a.to == b.to && a.cc == b.cc && a.bcc == b.bcc &&
        a.inReplyTo == b.inReplyTo && a.messageId == b.messageId;
}

bool operator==( const MailAddress& a, const MailAddress& b )
{
    return a.name == b.name && a.adl == b.adl && a.mailbox == b.mailbox && a.host == b.host;
}

void OneMessage::storeInterestingFields( Mailbox::TreeItemPart* p ) const
{
    p->setEncoding( bodyFldEnc.toLower() );
    p->setOctets( bodyFldOctets );
    bodyFldParam_t::const_iterator it = bodyFldParam.find( "CHARSET" );
    if ( it != bodyFldParam.end() ) {
        p->setCharset( *it );
    }
    if ( ! bodyFldDsp.first.isNull() ) {
        p->setBodyDisposition( bodyFldDsp.first );
        it = bodyFldDsp.second.find( "FILENAME" );
        if ( it != bodyFldDsp.second.end() ) {
            p->setFileName( Imap::decodeRFC2047String( *it ) );
        } else if ( ( it = bodyFldParam.find( "NAME" ) ) != bodyFldParam.end() ) {
            p->setFileName( Imap::decodeRFC2047String( *it ) );
        }
    }
    p->setBodyFldId( bodyFldId );
}

QList<Mailbox::TreeItem*> TextMessage::createTreeItems( Mailbox::TreeItem* parent ) const
{
    QList<Mailbox::TreeItem*> list;
    Mailbox::TreeItemPart* p = new Mailbox::TreeItemPart( parent, QString("%1/%2").arg( mediaType, mediaSubType) );
    storeInterestingFields( p );
    list << p;
    return list;
}

QList<Mailbox::TreeItem*> BasicMessage::createTreeItems( Mailbox::TreeItem* parent ) const
{
    QList<Mailbox::TreeItem*> list;
    Mailbox::TreeItemPart* p = new Mailbox::TreeItemPart( parent, QString("%1/%2").arg( mediaType, mediaSubType) );
    storeInterestingFields( p );
    list << p;
    return list;
}

QList<Mailbox::TreeItem*> MsgMessage::createTreeItems( Mailbox::TreeItem* parent ) const
{
    QList<Mailbox::TreeItem*> list;
    Mailbox::TreeItemPart* part = new Mailbox::TreeItemPart( parent, QString("%1/%2").arg( mediaType, mediaSubType) );
    part->setChildren( body->createTreeItems( part ) ); // always returns an empty list -> no need to qDeleteAll()
    storeInterestingFields( part );
    list << part;
    return list;
}

QList<Mailbox::TreeItem*> MultiMessage::createTreeItems( Mailbox::TreeItem* parent ) const
{
    // FIXME: store more data?
    QList<Mailbox::TreeItem*> list, list2;
    Mailbox::TreeItemPart* part = new Mailbox::TreeItemPart( parent, QString("multipart/%1").arg( mediaSubType) );
    for ( QList<QSharedPointer<AbstractMessage> >::const_iterator it = bodies.begin(); it != bodies.end(); ++it ) {
        list2 << (*it)->createTreeItems( part );
    }
    part->setChildren( list2 ); // always returns an empty list -> no need to qDeleteAll()
    list << part;
    return list;
}

}
}

QDebug operator<<( QDebug& dbg, const Imap::Message::Envelope& envelope )
{
    using namespace Imap::Message;
    return dbg << "Envelope( FROM" << MailAddress::prettyList( envelope.from, MailAddress::FORMAT_READABLE ) <<
            "TO" << MailAddress::prettyList( envelope.to, MailAddress::FORMAT_READABLE ) <<
            "CC" << MailAddress::prettyList( envelope.cc, MailAddress::FORMAT_READABLE ) <<
            "BCC" << MailAddress::prettyList( envelope.bcc, MailAddress::FORMAT_READABLE ) <<
            "SUBJECT" << envelope.subject <<
            "DATE" << envelope.date <<
            "MESSAGEID" << envelope.messageId;
}

QDataStream& operator>>( QDataStream& stream, Imap::Message::Envelope& e )
{
    return stream >> e.bcc >> e.cc >> e.date >> e.from >> e.inReplyTo >>
            e.messageId >> e.replyTo >> e.sender >> e.subject >> e.to;
}

QDataStream& operator<<( QDataStream& stream, const Imap::Message::Envelope& e )
{
    return stream << e.bcc << e.cc << e.date << e.from << e.inReplyTo <<
            e.messageId << e.replyTo << e.sender << e.subject << e.to;
}

QDataStream& operator>>( QDataStream& stream, Imap::Message::MailAddress& a )
{
    return stream >> a.adl >> a.host >> a.mailbox >> a.name;
}

QDataStream& operator<<( QDataStream& stream, const Imap::Message::MailAddress& a )
{
    return stream << a.adl << a.host << a.mailbox << a.name;
}
