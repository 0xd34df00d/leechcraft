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
#include <algorithm>
#include <QDebug>
#include <QStringList>
#include <QMutexLocker>
#include <QProcess>
#include <QTime>
#include <QTimer>
#include "Parser.h"
#include "Imap/Encoders.h"
#include "LowLevelParser.h"
#include "../../Streams/IODeviceSocket.h"

//#define PRINT_TRAFFIC 100
//#define PRINT_TRAFFIC_TX 500
//#define PRINT_TRAFFIC_RX 25
//#define PRINT_TRAFFIC_SENSITIVE

#ifdef PRINT_TRAFFIC
# ifndef PRINT_TRAFFIC_TX
#  define PRINT_TRAFFIC_TX PRINT_TRAFFIC
# endif
# ifndef PRINT_TRAFFIC_RX
#  define PRINT_TRAFFIC_RX PRINT_TRAFFIC
# endif
#endif

/*
 * Parser interface considerations:
 *
 * - Parser receives comments and gives back some kind of ID for tracking the
 *   command state
 * - "High-level stuff" like "has this command already finished" should be
 *   implemented on higher level
 * - Due to command pipelining, there's no way to find out that this untagged
 *   reply we just received was triggered by FOO command
 *
 * Interface:
 *
 * - One function per command
 * - Each received reply emits a signal (Qt-specific stuff now)
 *
 *
 * Usage example FIXME DRAFT:
 *
 *  Imap::Parser parser;
 *  Imap::CommandHandle res = parser.deleteFolder( "foo mailbox/bar/baz" );
 *
 *
 *
 * How it works under the hood:
 *
 * - When there are any data available on the net, process them ASAP
 * - When user queues a command, process it ASAP
 * - You can't block the caller of the queueCommand()
 *
 * So, how to implement this?
 *
 * - Whenever something interesting happens (data/command/exit
 *   requested/available), we ask the worker thread to do something
 *
 * */

namespace Imap {

Parser::Parser( QObject* parent, Socket* socket, const uint myId ):
        QObject(parent), _socket(socket), _lastTagUsed(0), _idling(false), _waitForInitialIdle(false),
        _literalPlus(false), _waitingForContinuation(false), _startTlsInProgress(false),
        _waitingForConnection(true), _readingMode(ReadingLine),
        _oldLiteralPosition(0), _parserId(myId)
{
    connect( _socket, SIGNAL( disconnected( const QString& ) ),
             this, SLOT( handleDisconnected( const QString& ) ) );
    connect( _socket, SIGNAL( readyRead() ), this, SLOT( handleReadyRead() ) );
    connect( _socket, SIGNAL(connected()), this, SLOT(handleConnectionEstablished()) );
    connect( _socket, SIGNAL(stateChanged(Imap::ConnectionState,QString)), this, SLOT(slotSocketStateChanged(Imap::ConnectionState,QString)) );
}

CommandHandle Parser::noop()
{
    return queueCommand( Commands::ATOM, "NOOP" );
}

CommandHandle Parser::logout()
{
    return queueCommand( Commands::Command( "LOGOUT") );
}

CommandHandle Parser::capability()
{
    // CAPABILITY should take precedence over LOGIN, because we have to check for LOGINDISABLED
    return queueCommand( Commands::Command() <<
                                          Commands::PartOfCommand( Commands::ATOM, "CAPABILITY" ) );
}

CommandHandle Parser::startTls()
{
    return queueCommand( Commands::Command() <<
                                          Commands::PartOfCommand( Commands::STARTTLS, "STARTTLS" ) );
}

#if 0
CommandHandle Parser::authenticate( /*Authenticator FIXME*/)
{
    // FIXME: needs higher priority
    return queueCommand( Commands::ATOM, "AUTHENTICATE" );
}
#endif

CommandHandle Parser::login( const QString& username, const QString& password )
{
    return queueCommand( Commands::Command( "LOGIN" ) <<
            Commands::PartOfCommand( username ) << Commands::PartOfCommand( password ) );
}

CommandHandle Parser::select( const QString& mailbox )
{
    return queueCommand( Commands::Command( "SELECT" ) << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::examine( const QString& mailbox )
{
    return queueCommand( Commands::Command( "EXAMINE" ) << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::deleteMailbox( const QString& mailbox )
{
    return queueCommand( Commands::Command( "DELETE" ) << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::create( const QString& mailbox )
{
    return queueCommand( Commands::Command( "CREATE" ) << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::rename( const QString& oldName, const QString& newName )
{
    return queueCommand( Commands::Command( "RENAME" ) <<
                         encodeImapFolderName( oldName ) <<
                         encodeImapFolderName( newName ) );
}

CommandHandle Parser::subscribe( const QString& mailbox )
{
    return queueCommand( Commands::Command( "SUBSCRIBE" ) << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::unSubscribe( const QString& mailbox )
{
    return queueCommand( Commands::Command( "UNSUBSCRIBE" ) << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::list( const QString& reference, const QString& mailbox )
{
    return queueCommand( Commands::Command( "LIST" ) << reference << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::lSub( const QString& reference, const QString& mailbox )
{
    return queueCommand( Commands::Command( "LSUB" ) << reference << encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::status( const QString& mailbox, const QStringList& fields )
{
    return queueCommand( Commands::Command( "STATUS" ) << encodeImapFolderName( mailbox ) <<
            Commands::PartOfCommand( Commands::ATOM, "(" + fields.join(" ") +")" )
            );
}

CommandHandle Parser::append( const QString& mailbox, const QString& message, const QStringList& flags, const QDateTime& timestamp )
{
    Commands::Command command( "APPEND" );
    command << encodeImapFolderName( mailbox );
    if ( flags.count() )
        command << Commands::PartOfCommand( Commands::ATOM, "(" + flags.join(" ") + ")" );
    if ( timestamp.isValid() )
        command << Commands::PartOfCommand( timestamp.toString() );
    command << Commands::PartOfCommand( Commands::LITERAL, message );

    return queueCommand( command );
}

CommandHandle Parser::check()
{
    return queueCommand( Commands::ATOM, "CHECK" );
}

CommandHandle Parser::close()
{
    return queueCommand( Commands::ATOM, "CLOSE" );
}

CommandHandle Parser::expunge()
{
    return queueCommand( Commands::ATOM, "EXPUNGE" );
}

CommandHandle Parser::_searchHelper( const QString& command, const QStringList& criteria, const QString& charset )
{
    Commands::Command cmd( command );

    if ( !charset.isEmpty() )
        cmd << "CHARSET" << charset;

    for ( QStringList::const_iterator it = criteria.begin(); it != criteria.end(); ++it )
        cmd << *it;

    return queueCommand( cmd );
}

CommandHandle Parser::uidSearchUid( const QString& sequence )
{
    Commands::Command command( "UID SEARCH" );
    command << Commands::PartOfCommand( Commands::ATOM, sequence );
    return queueCommand( command );
}

CommandHandle Parser::_sortHelper(const QString &command, const QStringList &sortCriteria, const QString &charset, const QStringList &searchCriteria)
{
    Q_ASSERT( ! sortCriteria.isEmpty() );
    Commands::Command cmd;

    cmd << Commands::PartOfCommand( Commands::ATOM, command ) <<
            Commands::PartOfCommand( Commands::ATOM, QString::fromAscii("(%1)").arg( sortCriteria.join( QString(' ') ) ) ) <<
            charset;

    for ( QStringList::const_iterator it = searchCriteria.begin(); it != searchCriteria.end(); ++it )
        cmd << *it;

    return queueCommand( cmd );
}

CommandHandle Parser::sort(const QStringList &sortCriteria, const QString &charset, const QStringList &searchCriteria)
{
    return _sortHelper( QLatin1String("SORT"), sortCriteria, charset, searchCriteria );
}

CommandHandle Parser::uidSort(const QStringList &sortCriteria, const QString &charset, const QStringList &searchCriteria)
{
    return _sortHelper( QLatin1String("UID SORT"), sortCriteria, charset, searchCriteria );
}

CommandHandle Parser::_threadHelper(const QString &command, const QString &algo, const QString &charset, const QStringList &searchCriteria)
{
    Commands::Command cmd;

    cmd << Commands::PartOfCommand( Commands::ATOM, command ) << algo << charset;

    for ( QStringList::const_iterator it = searchCriteria.begin(); it != searchCriteria.end(); ++it )
        cmd << *it;

    return queueCommand( cmd );
}

CommandHandle Parser::thread(const QString &algo, const QString &charset, const QStringList &searchCriteria)
{
    return _threadHelper( QLatin1String("THREAD"), algo, charset, searchCriteria );
}

CommandHandle Parser::uidThread(const QString &algo, const QString &charset, const QStringList &searchCriteria)
{
    return _threadHelper( QLatin1String("UID THREAD"), algo, charset, searchCriteria );
}

CommandHandle Parser::fetch( const Sequence& seq, const QStringList& items )
{
    return queueCommand( Commands::Command( "FETCH" ) <<
            Commands::PartOfCommand( Commands::ATOM, seq.toString() ) <<
            Commands::PartOfCommand( Commands::ATOM, '(' + items.join(" ") + ')' ) );
}

CommandHandle Parser::store( const Sequence& seq, const QString& item, const QString& value )
{
    return queueCommand( Commands::Command( "STORE" ) <<
            Commands::PartOfCommand( Commands::ATOM, seq.toString() ) <<
            Commands::PartOfCommand( Commands::ATOM, item ) <<
            Commands::PartOfCommand( Commands::ATOM, value )
            );
}

CommandHandle Parser::copy( const Sequence& seq, const QString& mailbox )
{
    return queueCommand( Commands::Command("COPY") <<
            Commands::PartOfCommand( Commands::ATOM, seq.toString() ) <<
            encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::uidFetch( const Sequence& seq, const QStringList& items )
{
    return queueCommand( Commands::Command( "UID FETCH" ) <<
            Commands::PartOfCommand( Commands::ATOM, seq.toString() ) <<
            Commands::PartOfCommand( Commands::ATOM, '(' + items.join(" ") + ')' ) );
}

CommandHandle Parser::uidStore( const Sequence& seq, const QString& item, const QString& value )
{
    return queueCommand( Commands::Command( "UID STORE" ) <<
            Commands::PartOfCommand( Commands::ATOM, seq.toString() ) <<
            Commands::PartOfCommand( Commands::ATOM, item ) <<
            Commands::PartOfCommand( Commands::ATOM, value ) );
}

CommandHandle Parser::uidCopy( const Sequence& seq, const QString& mailbox )
{
    return queueCommand( Commands::Command( "UID COPY" ) <<
            Commands::PartOfCommand( Commands::ATOM, seq.toString() ) <<
            encodeImapFolderName( mailbox ) );
}

CommandHandle Parser::xAtom( const Commands::Command& cmd )
{
    return queueCommand( cmd );
}

CommandHandle Parser::unSelect()
{
    return queueCommand( Commands::ATOM, "UNSELECT" );
}

CommandHandle Parser::idle()
{
    return queueCommand( Commands::IDLE, "IDLE" );
}

void Parser::idleDone()
{
    // This is not a new "command", so we don't go via queueCommand()
    // which would allocate a new tag for us, but submit directly
    Commands::Command cmd;
    cmd << Commands::PartOfCommand( Commands::IDLE_DONE, "DONE" );
    _cmdQueue.append( cmd );
    QTimer::singleShot( 0, this, SLOT(executeCommands()) );
}

void Parser::idleContinuationWontCome()
{
    Q_ASSERT(_waitForInitialIdle);
    _waitForInitialIdle = false;
    _idling = false;
    QTimer::singleShot( 0, this, SLOT(executeCommands()) );
}

void Parser::idleMagicallyTerminatedByServer()
{
    Q_ASSERT( ! _waitForInitialIdle );
    Q_ASSERT( _idling );
    _idling = false;
}

CommandHandle Parser::namespaceCommand()
{
    return queueCommand( Commands::ATOM, "NAMESPACE" );
}

CommandHandle Parser::queueCommand( Commands::Command command )
{
    QString tag = generateTag();
    command.addTag( tag );
    _cmdQueue.append( command );
    QTimer::singleShot( 0, this, SLOT(executeCommands()) );
    return tag;
}

void Parser::queueResponse( const QSharedPointer<Responses::AbstractResponse>& resp )
{
    _respQueue.push_back( resp );
    emit responseReceived( this );
}

bool Parser::hasResponse() const
{
    return ! _respQueue.empty();
}

QSharedPointer<Responses::AbstractResponse> Parser::getResponse()
{
    QSharedPointer<Responses::AbstractResponse> ptr;
    if ( _respQueue.empty() )
        return ptr;
    ptr = _respQueue.front();
    _respQueue.pop_front();
    return ptr;
}

QString Parser::generateTag()
{
    return QString( "y%1" ).arg( _lastTagUsed++ );
}

void Parser::handleReadyRead()
{
    while ( 1 ) {
        switch ( _readingMode ) {
            case ReadingLine:
                if ( _socket->canReadLine() ) {
                    reallyReadLine();
                } else {
                    // Not enough data yet, let's try again later
                    return;
                }
                break;
            case ReadingNumberOfBytes:
                {
                    QByteArray buf = _socket->read( _readingBytes );
                    _readingBytes -= buf.size();
                    _currentLine += buf;
                    if ( _readingBytes == 0 ) {
                        // we've read the literal
                        _readingMode = ReadingLine;
                    } else {
                        return;
                    }
                }
                break;
        }
    }
}

void Parser::reallyReadLine()
{
    try {
        _currentLine += _socket->readLine();
        if ( _currentLine.endsWith( "}\r\n" ) ) {
            int offset = _currentLine.lastIndexOf( '{' );
            if ( offset < _oldLiteralPosition )
                throw ParseError( "Got unmatched '}'", _currentLine, _currentLine.size() - 3 );
            bool ok;
            int number = _currentLine.mid( offset + 1, _currentLine.size() - offset - 4 ).toInt( &ok );
            if ( !ok )
                throw ParseError( "Can't parse numeric literal size", _currentLine, offset );
            if ( number < 0 )
                throw ParseError( "Negative literal size", _currentLine, offset );
            _oldLiteralPosition = offset;
            _readingMode = ReadingNumberOfBytes;
            _readingBytes = number;
        } else if ( _currentLine.endsWith( "\r\n" ) ) {
            // it's complete
            if ( _startTlsInProgress && _currentLine.startsWith( _startTlsCommand ) ) {
                _startTlsCommand.clear();
                _startTlsReply = _currentLine;
                _currentLine.clear();
                _oldLiteralPosition = 0;
                QTimer::singleShot( 0, this, SLOT(finishStartTls()) );
                return;
            }
            processLine( _currentLine );
            _currentLine.clear();
            _oldLiteralPosition = 0;
        } else {
            throw CantHappen( "canReadLine() returned true, but following readLine() failed" );
        }
    } catch ( ParserException& e ) {
        emit parseError( this, QString::fromStdString( e.exceptionClass() ), QString::fromStdString( e.msg() ), e.line(), e.offset() );
    }
}

void Parser::executeCommands()
{
    while ( ! _waitingForContinuation && ! _waitForInitialIdle &&
            ! _waitingForConnection &&
            ! _cmdQueue.isEmpty() && ! _startTlsInProgress )
        executeACommand();
}

void Parser::finishStartTls()
{
    emit lineSent( this, "*** STARTTLS" );
#ifdef PRINT_TRAFFIC_TX
    qDebug() << _parserId << "*** STARTTLS";
#endif
    _cmdQueue.pop_front();
    _socket->startTls(); // warn: this might invoke event loop
    _startTlsInProgress = false;
    processLine( _startTlsReply );
}

void Parser::executeACommand()
{
    Q_ASSERT( ! _cmdQueue.isEmpty() );
    Commands::Command& cmd = _cmdQueue.first();

    QByteArray buf;

    bool sensitiveCommand = ( cmd._cmds.size() > 2 && cmd._cmds[1]._text == QLatin1String("LOGIN"));
    QByteArray privateMessage = sensitiveCommand ? QByteArray("[LOGIN command goes here]") : QByteArray();

#ifdef PRINT_TRAFFIC
#ifdef PRINT_TRAFFIC_SENSITIVE
    bool printThisCommand = true;
#else
    bool printThisCommand = ! sensitiveCommand;
#endif
#endif


    if ( cmd._cmds[ cmd._currentPart ]._kind == Commands::ATOM )
        emit sendingCommand( this, cmd._cmds[ cmd._currentPart ]._text );

    if ( cmd._cmds[ cmd._currentPart ]._kind == Commands::IDLE_DONE ) {
        // Handling of the IDLE_DONE is a bit special, as we have to check and update the _idling flag...
        Q_ASSERT( _idling );
        buf.append( "DONE\r\n" );
#ifdef PRINT_TRAFFIC_TX
        qDebug() << _parserId << ">>>" << buf.left( PRINT_TRAFFIC_TX ).trimmed();
#endif
        _socket->write( buf );
        _idling = false;
        _cmdQueue.pop_front();
        emit lineSent( this, buf );
        buf.clear();
        return;
    }

    Q_ASSERT( ! _idling );

    while ( 1 ) {
        Commands::PartOfCommand& part = cmd._cmds[ cmd._currentPart ];
        switch( part._kind ) {
            case Commands::ATOM:
                buf.append( part._text );
                break;
            case Commands::QUOTED_STRING:
                {
                QString item = part._text;
                item.replace( QChar('\\'), QString::fromAscii("\\\\") );
                buf.append( '"' );
                buf.append( item );
                buf.append( '"' );
                }
                break;
            case Commands::LITERAL:
                if ( _literalPlus ) {
                    buf.append( '{' );
                    buf.append( QByteArray::number( part._text.size() ) );
                    buf.append( "+}\r\n" );
                    buf.append( part._text );
                } else if ( part._numberSent ) {
                    buf.append( part._text );
                } else {
                    buf.append( '{' );
                    buf.append( QByteArray::number( part._text.size() ) );
                    buf.append( "}\r\n" );
#ifdef PRINT_TRAFFIC_TX
                    if ( printThisCommand )
                        qDebug() << _parserId << ">>>" << buf.left( PRINT_TRAFFIC_TX ).trimmed();
                    else
                        qDebug() << _parserId << ">>> [sensitive command] -- added literal";
#endif
                    _socket->write( buf );
                    part._numberSent = true;
                    _waitingForContinuation = true;
                    emit lineSent( this, sensitiveCommand ? privateMessage : buf );
                    return; // and wait for continuation request
                }
                break;
            case Commands::IDLE_DONE:
                Q_ASSERT(false); // is handled above
                break;
            case Commands::IDLE:
                buf.append( "IDLE\r\n" );
#ifdef PRINT_TRAFFIC_TX
                qDebug() << _parserId << ">>>" << buf.left( PRINT_TRAFFIC_TX ).trimmed();
#endif
                _socket->write( buf );
                _idling = true;
                _waitForInitialIdle = true;
                _cmdQueue.pop_front();
                emit lineSent( this, buf );
                return;
                break;
            case Commands::STARTTLS:
                _startTlsCommand = buf;
                buf.append( "STARTTLS\r\n" );
#ifdef PRINT_TRAFFIC_TX
                qDebug() << _parserId << ">>>" << buf.left( PRINT_TRAFFIC_TX ).trimmed();
#endif
                _socket->write( buf );
                _startTlsInProgress = true;
                emit lineSent( this, buf );
                return;
                break;
        }
        if ( cmd._currentPart == cmd._cmds.size() - 1 ) {
            // finalize
            buf.append( "\r\n" );
#ifdef PRINT_TRAFFIC_TX
            if ( printThisCommand )
                qDebug() << _parserId << ">>>" << buf.left( PRINT_TRAFFIC_TX ).trimmed();
            else
                qDebug() << _parserId << ">>> [sensitive command]";
#endif
            _socket->write( buf );
            _cmdQueue.pop_front();
            emit lineSent( this, sensitiveCommand ? privateMessage : buf );
            break;
        } else {
            buf.append( ' ' );
            ++cmd._currentPart;
        }
    }
}

/** @short Process a line from IMAP server */
void Parser::processLine( QByteArray line )
{
#ifdef PRINT_TRAFFIC_RX
    QByteArray debugLine = line.trimmed();
    if ( debugLine.size() > PRINT_TRAFFIC_RX )
        qDebug() << _parserId << "<<<" << debugLine.left( PRINT_TRAFFIC_RX ) << "...";
    else
        qDebug() << _parserId << "<<<" << debugLine;
#endif
    emit lineReceived( this, line );
    if ( line.startsWith( "* " ) ) {
        queueResponse( parseUntagged( line ) );
    } else if ( line.startsWith( "+ " ) ) {
        if ( _waitingForContinuation ) {
            _waitingForContinuation = false;
            QTimer::singleShot( 0, this, SLOT(executeCommands()) );
        } else if ( _waitForInitialIdle ) {
            _waitForInitialIdle = false;
            QTimer::singleShot( 0, this, SLOT(executeCommands()) );
        } else {
            throw ContinuationRequest( line.constData() );
        }
    } else {
        queueResponse( parseTagged( line ) );
    }
}

QSharedPointer<Responses::AbstractResponse> Parser::parseUntagged( const QByteArray& line )
{
    int pos = 2;
    uint number;
    try {
        number = LowLevelParser::getUInt( line, pos );
        ++pos;
    } catch ( ParseError& ) {
        return _parseUntaggedText( line, pos );
    }
    return _parseUntaggedNumber( line, pos, number );
}

QSharedPointer<Responses::AbstractResponse> Parser::_parseUntaggedNumber(
        const QByteArray& line, int& start, const uint number )
{
    if ( start == line.size() )
        // number and nothing else
        throw NoData( line, start );

    QByteArray kindStr = LowLevelParser::getAtom( line, start );
    Responses::Kind kind;
    try {
        kind = Responses::kindFromString( kindStr );
    } catch ( UnrecognizedResponseKind& e ) {
        throw UnrecognizedResponseKind( e.what(), line, start );
    }

    switch ( kind ) {
        case Responses::EXISTS:
        case Responses::RECENT:
        case Responses::EXPUNGE:
            // no more data should follow
            if ( start >= line.size() )
                throw TooMuchData( line, start );
            else if ( line.mid(start) != QByteArray( "\r\n" ) )
                throw UnexpectedHere( line, start ); // expected CRLF
            else
                try {
                    return QSharedPointer<Responses::AbstractResponse>(
                            new Responses::NumberResponse( kind, number ) );
                } catch ( UnexpectedHere& e ) {
                    throw UnexpectedHere( e.what(), line, start );
                }
            break;

        case Responses::FETCH:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Fetch( number, line, start ) );
            break;

        default:
            break;
    }
    throw UnexpectedHere( line, start );
}

QSharedPointer<Responses::AbstractResponse> Parser::_parseUntaggedText(
        const QByteArray& line, int& start )
{
    Responses::Kind kind;
    try {
        kind = Responses::kindFromString( LowLevelParser::getAtom( line, start ) );
    } catch ( UnrecognizedResponseKind& e ) {
        throw UnrecognizedResponseKind( e.what(), line, start );
    }
    ++start;
    if ( start == line.size() && kind != Responses::SEARCH && kind != Responses::SORT )
        throw NoData( line, start );
    switch ( kind ) {
        case Responses::CAPABILITY:
            {
                QStringList capabilities;
                QList<QByteArray> list = line.mid( start ).split( ' ' );
                for ( QList<QByteArray>::const_iterator it = list.begin(); it != list.end(); ++it ) {
                    QByteArray str = *it;
                    if ( str.endsWith( "\r\n" ) )
                        str.chop(2);
                    capabilities << str;
                }
                if ( !capabilities.count() )
                    throw NoData( line, start );
                return QSharedPointer<Responses::AbstractResponse>(
                        new Responses::Capability( capabilities ) );
            }
        case Responses::OK:
        case Responses::NO:
        case Responses::BAD:
        case Responses::PREAUTH:
        case Responses::BYE:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::State( QString::null, kind, line, start ) );
        case Responses::LIST:
        case Responses::LSUB:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::List( kind, line, start ) );
        case Responses::FLAGS:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Flags( line, start ) );
        case Responses::SEARCH:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Search( line, start ) );
        case Responses::STATUS:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Status( line, start ) );
        case Responses::NAMESPACE:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Namespace( line, start ) );
        case Responses::SORT:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Sort( line, start ) );
        case Responses::THREAD:
            return QSharedPointer<Responses::AbstractResponse>(
                    new Responses::Thread( line, start ) );

        // Those already handled above follow here
        case Responses::EXPUNGE:
        case Responses::FETCH:
        case Responses::EXISTS:
        case Responses::RECENT:
            throw UnexpectedHere("Mallformed response: the number should go first", line, start);
    }
    throw UnexpectedHere( line, start );
}

QSharedPointer<Responses::AbstractResponse> Parser::parseTagged( const QByteArray& line )
{
    int pos = 0;
    const QByteArray tag = LowLevelParser::getAtom( line, pos );
    ++pos;
    const Responses::Kind kind = Responses::kindFromString( LowLevelParser::getAtom( line, pos ) );
    ++pos;

    return QSharedPointer<Responses::AbstractResponse>(
            new Responses::State( tag, kind, line, pos ) );
}

void Parser::enableLiteralPlus( const bool enabled )
{
    _literalPlus = enabled;
}

void Parser::handleDisconnected( const QString& reason )
{
    emit lineReceived( this, "*** Socket disconnected: " + reason.toLocal8Bit() );
#ifdef PRINT_TRAFFIC_TX
    qDebug() << _parserId << "*** Socket disconnected";
#endif
    emit disconnected( this, reason );
}

void Parser::handleConnectionEstablished()
{
#ifdef PRINT_TRAFFIC_TX
    qDebug() << _parserId << "*** Connection established";
#endif
    emit lineReceived( this, "*** Connection established" );
    _waitingForConnection = false;
    QTimer::singleShot( 0, this, SLOT(executeCommands()));
}

Parser::~Parser()
{
    // We want to prevent nasty signals from the underlying socket from
    // interfering with this object -- some of our local data might have
    // been already destroyed!
    _socket->disconnect( this );
    _socket->deleteLater();
}

uint Parser::parserId() const
{
    return _parserId;
}

void Parser::slotSocketStateChanged( const Imap::ConnectionState connState, const QString &message )
{
    emit lineReceived( this, "*** " + message.toLocal8Bit());
    emit connectionStateChanged( this, connState );
}

Sequence::Sequence( const uint num ): _kind(DISTINCT)
{
    _list << num;
}

Sequence Sequence::startingAt( const uint lo )
{
    Sequence res( lo );
    res._lo = lo;
    res._kind = UNLIMITED;
    return res;
}

QString Sequence::toString() const
{
    switch ( _kind ) {
        case DISTINCT:
        {
            Q_ASSERT( ! _list.isEmpty() );

            QStringList res;
            int i = 0;
            while ( i < _list.size() ) {
                int old = i;
                while ( i < _list.size() - 1 &&
                        _list[i] == _list[ i + 1 ] - 1 )
                    ++i;
                if ( old != i ) {
                    // we've found a sequence
                    res << QString::number( _list[old] ) + QLatin1Char(':') + QString::number( _list[i] );
                } else {
                    res << QString::number( _list[i] );
                }
                ++i;
            }
            return res.join( QLatin1String(",") );
            break;
        }
        case RANGE:
            Q_ASSERT( _lo <= _hi );
            if ( _lo == _hi )
                return QString::number( _lo );
            else
                return QString::number( _lo ) + QLatin1Char(':') + QString::number( _hi );
        case UNLIMITED:
            return QString::number( _lo ) + QLatin1String(":*");
    }
    // fix gcc warning
    Q_ASSERT( false );
    return QString();
}

Sequence& Sequence::add( uint num )
{
    Q_ASSERT( _kind == DISTINCT );
    QList<uint>::iterator it = qLowerBound( _list.begin(), _list.end(), num );
    if ( it == _list.end() || *it != num )
        _list.insert( it, num );
    return *this;
}

Sequence Sequence::fromList(QList<uint> numbers)
{
    Q_ASSERT(!numbers.isEmpty());
    qSort(numbers);
    Sequence seq(numbers.first());
    for (int i = 1; i < numbers.size(); ++i) {
        seq.add(numbers[i]);
    }
    return seq;
}

QTextStream& operator<<( QTextStream& stream, const Sequence& s )
{
    return stream << s.toString();
}

}
