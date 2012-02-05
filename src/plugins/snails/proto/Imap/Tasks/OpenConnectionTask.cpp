/* Copyright (C) 2007 - 2011 Jan Kundrát <jkt@flaska.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "OpenConnectionTask.h"
#include <QTimer>

namespace Imap {
namespace Mailbox {

OpenConnectionTask::OpenConnectionTask( Model* _model ) :
    ImapTask( _model ), waitingForGreetings(true), gotPreauth(false)
{
    // FIXME: honor the offline policy here
    parser = new Parser( model, model->_socketFactory->create(), ++model->lastParserId );
    Model::ParserState parserState = Model::ParserState( parser );
    connect( parser, SIGNAL(responseReceived(Imap::Parser*)), model, SLOT(responseReceived(Imap::Parser*)) );
    connect( parser, SIGNAL(disconnected(Imap::Parser*,const QString)), model, SLOT(slotParserDisconnected(Imap::Parser*,const QString)) );
    connect( parser, SIGNAL(connectionStateChanged(Imap::Parser*,Imap::ConnectionState)), model, SLOT(handleSocketStateChanged(Imap::Parser*,Imap::ConnectionState)) );
    connect( parser, SIGNAL(sendingCommand(Imap::Parser*,QString)), model, SLOT(parserIsSendingCommand(Imap::Parser*,QString)) );
    connect( parser, SIGNAL(parseError(Imap::Parser*,QString,QString,QByteArray,int)), model, SLOT(slotParseError(Imap::Parser*,QString,QString,QByteArray,int)) );
    connect( parser, SIGNAL(lineReceived(Imap::Parser*,QByteArray)), model, SLOT(slotParserLineReceived(Imap::Parser*,QByteArray)) );
    connect( parser, SIGNAL(lineSent(Imap::Parser*,QByteArray)), model, SLOT(slotParserLineSent(Imap::Parser*,QByteArray)) );
    if ( model->_startTls ) {
        startTlsCmd = parser->startTls();
        emit model->activityHappening( true );
    }
    parserState.activeTasks.append( this );
    model->_parsers[ parser ] = parserState;
}

OpenConnectionTask::OpenConnectionTask( Model* _model, void* dummy): ImapTask( _model )
{
    Q_UNUSED( dummy );
}

void OpenConnectionTask::perform()
{
    // nothing should happen here
}

/** @short Process the "state" response originating from the IMAP server */
bool OpenConnectionTask::handleStateHelper( const Imap::Responses::State* const resp )
{
    if ( waitingForGreetings ) {
        handleInitialResponse(resp);
        return true;
    }

    if ( resp->tag.isEmpty() )
        return false;

    if ( resp->tag == capabilityCmd ) {
        if ( resp->kind == Responses::OK ) {
            if ( gotPreauth ) {
                // The greetings indicated that we're already in the auth state, and now we
                // know capabilities, too, so we're done here
                _completed();
            } else {
                // We want to log in, but we might have to STARTTLS before
                if ( model->accessParser( parser ).capabilities.contains( QLatin1String("LOGINDISABLED") ) ) {
                    log("Can't login yet, trying STARTTLS", LOG_OTHER);
                    // ... and we are forbidden from logging in, so we have to try the STARTTLS
                    startTlsCmd = parser->startTls();
                    emit model->activityHappening( true );
                } else {
                    // Apparently no need for STARTTLS and we are free to login
                    loginCmd = model->performAuthentication( parser );
                }
            }
        } else {
            // FIXME: Tasks API error handling
        }
        return true;
    } else if ( resp->tag == loginCmd ) {
        // The LOGIN command is finished, and we know capabilities already
        Q_ASSERT( model->accessParser( parser ).capabilitiesFresh );
        if ( resp->kind == Responses::OK ) {
            model->changeConnectionState( parser, CONN_STATE_AUTHENTICATED);
            _completed();
        } else {
            QString message;
            switch ( resp->respCode ) {
            case Responses::UNAVAILABLE:
                message = tr("Temporary failure because a subsystem is down.");
                break;
            case Responses::AUTHENTICATIONFAILED:
                message = tr("Authentication failed for some reason on which the server is "
                             "unwilling to elaborate.  Typically, this includes \"unknown "
                             "user\" and \"bad password\".");
                break;
            case Responses::AUTHORIZATIONFAILED:
                message = tr("Authentication succeeded in using the authentication identity, "
                             "but the server cannot or will not allow the authentication "
                             "identity to act as the requested authorization identity.");
                break;
            case Responses::EXPIRED:
                message = tr("Either authentication succeeded or the server no longer had the "
                             "necessary data; either way, access is no longer permitted using "
                             "that passphrase.  You should get a new passphrase.");
                break;
            case Responses::PRIVACYREQUIRED:
                message = tr("he operation is not permitted due to a lack of privacy.");
                break;
            case Responses::CONTACTADMIN:
                message = tr("You should contact the system administrator or support desk.");
                break;
            default:
                break;
            }

            if ( message.isEmpty() ) {
                message = tr("Login failed: %1").arg(resp->message);
            } else {
                message = tr("%1\r\n\r\n%2").arg(message, resp->message);
            }
            model->emitAuthFailed(message);
            loginCmd = model->performAuthentication( parser );

            // FIXME: error handling
        }
        return true;
    } else if ( resp->tag == startTlsCmd ) {
        // So now we've got a secure connection, but we will have to login. Additionally,
        // we are obliged to forget any capabilities.
        model->accessParser( parser ).capabilitiesFresh = false;
        // FIXME: why do I have to comment that out?
        if ( resp->kind == Responses::OK ) {
            capabilityCmd = parser->capability();
            emit model->activityHappening( true );
        } else {
            // Well, this place is *very* bad -- we're in the middle of a responseRecevied(), Model is iterating over active tasks
            // and we really want to emit that connectionError signal here. The problem is that a typical reaction from the GUI is
            // to show a dialog box, which unfortunately invokes the event loop, which would in turn handle the socketDisconnected()
            // (because the real SSL operation for switching on the encryption failed, too).
            // Let's just throw an exception and let the Model deal with it.
            throw StartTlsFailed( tr("Can't establish a secure connection to the server (STARTTLS failed: %1). Refusing to proceed.").arg( resp->message ).toUtf8().constData() );
        }
        model->parsersMightBeIdling();
        return true;
    } else {
        return false;
    }
}

/** @short Helper for dealing with the very first response from the server */
void OpenConnectionTask::handleInitialResponse( const Imap::Responses::State* const resp )
{
    waitingForGreetings = false;
    if ( ! resp->tag.isEmpty() ) {
        throw Imap::UnexpectedResponseReceived(
                "Waiting for initial OK/BYE/PREAUTH, but got tagged response instead",
                *resp );
    }

    using namespace Imap::Responses;
    switch ( resp->kind ) {
    case PREAUTH:
        {
            // Cool, we're already authenticated. Now, let's see if we have to issue CAPABILITY or if we already know that
            gotPreauth = true;
            model->changeConnectionState( parser, CONN_STATE_AUTHENTICATED);
            if ( ! model->accessParser( parser ).capabilitiesFresh ) {
                capabilityCmd = parser->capability();
                emit model->activityHappening( true );
            } else {
                _completed();
            }
            break;
        }
    case OK:
        if ( model->_startTls ) {
            // The STARTTLS command is already queued -> no need to issue it once again
        } else {
            // The STARTTLS surely has not been issued yet
            if ( ! model->accessParser( parser ).capabilitiesFresh ) {
                capabilityCmd = parser->capability();
                emit model->activityHappening( true );
            } else if ( model->accessParser( parser ).capabilities.contains( QLatin1String("LOGINDISABLED") ) ) {
                log("Can't login yet, trying STARTTLS", LOG_OTHER);
                // ... and we are forbidden from logging in, so we have to try the STARTTLS
                startTlsCmd = parser->startTls();
                emit model->activityHappening( true );
            } else {
                // Apparently no need for STARTTLS and we are free to login
                loginCmd = model->performAuthentication( parser );
            }
        }
        break;
    case BYE:
        model->changeConnectionState( parser, CONN_STATE_LOGOUT );
        // FIXME: Tasks error handling
        break;
    case BAD:
        // If it was an ALERT, we've already warned the user
        if ( resp->respCode != ALERT ) {
            emit model->alertReceived( tr("The server replied with the following BAD response:\n%1").arg( resp->message ) );
        }
        // FIXME: Tasks error handling
        break;
    default:
        throw Imap::UnexpectedResponseReceived(
                "Waiting for initial OK/BYE/BAD/PREAUTH, but got this instead",
                *resp );
    }
}

}
}
