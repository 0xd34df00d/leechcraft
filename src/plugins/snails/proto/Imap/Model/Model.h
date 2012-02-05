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

#ifndef IMAP_MODEL_H
#define IMAP_MODEL_H

#include <QAbstractItemModel>
#include <QPointer>
#include <QTimer>
#include "Cache.h"
#include "../ConnectionState.h"
#include "../Parser/Parser.h"
#include "Streams/SocketFactory.h"
#include "CopyMoveOperation.h"
#include "TaskFactory.h"

#include "Logging.h"

class QAuthenticator;

class FakeCapabilitiesInjector;
class ImapModelIdleTest;

/** @short Namespace for IMAP interaction */
namespace Imap {

/** @short Classes for handling of mailboxes and connections */
namespace Mailbox {

class TreeItem;
class TreeItemMailbox;
class TreeItemMsgList;
class TreeItemMessage;
class TreeItemPart;
class MsgListModel;
class MailboxModel;
class DelayedAskForChildrenOfMailbox;

class ImapTask;
class KeepMailboxOpenTask;

/** @short Progress of mailbox synchronization with the IMAP server */
typedef enum { STATE_WAIT_FOR_CONN, /**< Waiting for connection to become active */
               STATE_SELECTING, /**< SELECT command in progress */
               STATE_SYNCING_UIDS, /**< UID syncing in progress */
               STATE_SYNCING_FLAGS, /**< Flag syncing in progress */
               STATE_DONE /**< Mailbox is fully synchronized, both UIDs and flags are up to date */
           } MailboxSyncingProgress;

/** @short A model implementing view of the whole IMAP server */
class Model: public QAbstractItemModel {
    Q_OBJECT

    /** @short How to open a mailbox */
    enum RWMode {
        ReadOnly /**< @short Use EXAMINE or leave it in SELECTed mode*/,
        ReadWrite /**< @short Invoke SELECT if necessarry */
    };

    /** @short Helper structure for keeping track of each parser's state */
    struct ParserState {
        /** @short Which parser are we talking about here */
        QPointer<Parser> parser;
        /** @short The mailbox which we'd like to have selected */
        ConnectionState connState;
        /** @short The logout command */
        CommandHandle logoutCmd;
        /** @short List of tasks which are active already, and should therefore receive events */
        QList<ImapTask*> activeTasks;
        /** @short An active KeepMailboxOpenTask, if one exists */
        KeepMailboxOpenTask* maintainingTask;
        /** @short A list of cepabilities, as advertised by the server */
        QStringList capabilities;
        /** @short Is the @arg capabilities usable? */
        bool capabilitiesFresh;
        /** @short LIST responses which were not processed yet */
        QList<Responses::List> listResponses;

        ParserState( Parser* _parser ): parser(_parser), connState(CONN_STATE_NONE), maintainingTask(0), capabilitiesFresh(false) {}
        ParserState(): connState(CONN_STATE_NONE), maintainingTask(0), capabilitiesFresh(false) {}
    };

    /** @short Policy for accessing network */
    enum NetworkPolicy {
        /**< @short No access to the network at all

        All network activity is suspended. If an action requires network access,
        it will either fail or be queued for later. */
        NETWORK_OFFLINE,
        /** @short Connections are possible, but expensive

        Information that is cached is preferred, as long as it is usable.
        Trojita will never miss a mail in this mode, but for example it won't
        check for new mailboxes. */
        NETWORK_EXPENSIVE,
        /** @short Connections have zero cost

        Normal mode of operation. All network activity is assumed to have zero
        cost and Trojita is free to ask network as often as possible. It will
        still use local cache when it makes sense, though. */
        NETWORK_ONLINE
    };

    mutable AbstractCache* _cache;
    mutable SocketFactoryPtr _socketFactory;
    TaskFactoryPtr _taskFactory;
    mutable QMap<Parser*,ParserState> _parsers;
    int _maxParsers;
    mutable TreeItemMailbox* _mailboxes;
    mutable NetworkPolicy _netPolicy;
    bool _startTls;

    mutable QList<Imap::Responses::NamespaceData> _personalNamespace, _otherUsersNamespace, _sharedNamespace;


public:
    Model( QObject* parent, AbstractCache* cache, SocketFactoryPtr socketFactory, TaskFactoryPtr taskFactory, bool offline );
    ~Model();

    virtual QModelIndex index(int row, int column, const QModelIndex& parent ) const;
    virtual QModelIndex parent(const QModelIndex& index ) const;
    virtual int rowCount(const QModelIndex& index ) const;
    virtual int columnCount(const QModelIndex& index ) const;
    virtual QVariant data(const QModelIndex& index, int role ) const;
    virtual bool hasChildren( const QModelIndex& parent = QModelIndex() ) const;

    void handleState( Imap::Parser* ptr, const Imap::Responses::State* const resp );
    void handleCapability( Imap::Parser* ptr, const Imap::Responses::Capability* const resp );
    void handleNumberResponse( Imap::Parser* ptr, const Imap::Responses::NumberResponse* const resp );
    void handleList( Imap::Parser* ptr, const Imap::Responses::List* const resp );
    void handleFlags( Imap::Parser* ptr, const Imap::Responses::Flags* const resp );
    void handleSearch( Imap::Parser* ptr, const Imap::Responses::Search* const resp );
    void handleStatus( Imap::Parser* ptr, const Imap::Responses::Status* const resp );
    void handleFetch( Imap::Parser* ptr, const Imap::Responses::Fetch* const resp );
    void handleNamespace( Imap::Parser* ptr, const Imap::Responses::Namespace* const resp );
    void handleSort( Imap::Parser* ptr, const Imap::Responses::Sort* const resp );
    void handleThread( Imap::Parser* ptr, const Imap::Responses::Thread* const resp );

    AbstractCache* cache() const { return _cache; }
    /** Throw away current cache implementation, replace it with the new one

The old cache is automatically deleted.
*/
    void setCache( AbstractCache* cache );

    /** @short Force a SELECT / EXAMINE of a mailbox

This command sends a SELECT or EXAMINE command to the remote server, even if the
requested mailbox is currently selected. This has a side effect that we synchronize
the list of messages, which is why this function exists in the first place.
*/
    void resyncMailbox( const QModelIndex& mbox );

    /** @short Ask the server to set/unset the \\Deleted flag for the indicated messages */
    void markMessagesDeleted(const QModelIndexList &messages, bool marked);
    /** @short Ask the server to set/unset the \\Seen flag for the indicated messages */
    void markMessagesRead(const QModelIndexList &messages, bool marked);

    /** @short Run the EXPUNGE command in the specified mailbox */
    void expungeMailbox( TreeItemMailbox* mbox );

    /** @short Copy or move a sequence of messages between two mailboxes */
    void copyMoveMessages( TreeItemMailbox* sourceMbox, const QString& destMboxName, QList<uint> uids, const CopyMoveOperation op );

    /** @short Create a new mailbox */
    void createMailbox( const QString& name );
    /** @short Delete an existing mailbox */
    void deleteMailbox( const QString& name );

    /** @short Returns true if we are allowed to access the network */
    bool isNetworkAvailable() const { return _netPolicy != NETWORK_OFFLINE; }
    /** @short Returns true if the network access is cheap */
    bool isNetworkOnline() const { return _netPolicy == NETWORK_ONLINE; }

    /** @short Return a TreeItem* for a specified index

Certain proxy models implement their own indexes. These indexes typically won't
share the internalPointer() with the original model.  Because we use this pointer
quite often, a method is needed to automatically go through the list of all proxy
models and return the appropriate raw pointer.

Upon success, a valid pointer is returned, *whichModel is set to point to the
corresponding Model instance and *translatedIndex contains the index in the real
underlying model. If any of these pointers is NULL, it won't get changed, of course.

Upon failure, this function returns 0 and doesn't touch any of @arg whichModel
and @arg translatedIndex.
*/
    static TreeItem* realTreeItem( QModelIndex index, const Model** whichModel = 0, QModelIndex* translatedIndex = 0 );

    /** @short Walks the index hierarchy up until it finds a message which owns this part/message */
    static QModelIndex findMessageForItem( QModelIndex index );

    /** @short Inform the model that data for this message won't likely be requested in near future

Model will transform the corresponding TreeItemMessage into the state similar to how it would look
right after a fresh mailbox synchronization. All TreeItemParts will be freed, envelope and body
structure forgotten. This will substantially reduce Model's memory usage.

The UID and flags are not affected by this operation. The cache and any data stored in there will
also be left intact (and would indeed be consulted instead of the network when future requests for
this message happen again.

*/
    void releaseMessageData( const QModelIndex &message );

    /** @short Return a list of capabilities which are supported by the server */
    QStringList capabilities() const;

    /** @short Log an IMAP-related message */
    void logTrace(uint parserId, const LogKind kind, const QString &source, const QString &message);

public slots:
    /** @short Ask for an updated list of mailboxes on the server */
    void reloadMailboxList();

    /** @short Set the netowrk access policy to "no access allowed" */
    void setNetworkOffline() { setNetworkPolicy( NETWORK_OFFLINE ); }
    /** @short Set the network access policy to "possible, but expensive" */
    void setNetworkExpensive() { setNetworkPolicy( NETWORK_EXPENSIVE ); }
    /** @short Set the network access policy to "it's cheap to use it" */
    void setNetworkOnline() { setNetworkPolicy( NETWORK_ONLINE ); }

    /** @short Try to maintain a connection to the given mailbox

      This function informs the Model that the user is interested in receiving
      updates about the mailbox state, such as about the arrival of new messages.
      The usual response to such a hint is launching the IDLE command.
    */
    void switchToMailbox( const QModelIndex& mbox );

private slots:
    /** @short Handler for the "parser got disconnected" event */
    void slotParserDisconnected( Imap::Parser *parser, const QString );

    /** @short Parser throwed out an exception */
    void slotParseError( Imap::Parser *parser, const QString& exceptionClass, const QString& errorMessage, const QByteArray& line, int position );

    /** @short Helper for low-level state change propagation */
    void handleSocketStateChanged( Imap::Parser *parser, Imap::ConnectionState state );

    /** @short Handler for the Parser::sendingCommand() signal */
    void parserIsSendingCommand( Imap::Parser *parser, const QString& tag );

    /** @short The parser has received a full line */
    void slotParserLineReceived( Imap::Parser *parser, const QByteArray& line );

    /** @short The parser has sent a block of data */
    void slotParserLineSent( Imap::Parser *parser, const QByteArray& line );

    /** @short There's been a change in the state of various tasks */
    void slotTasksChanged();

    /** @short A maintaining task is about to die */
    void slotTaskDying( QObject *obj );

signals:
    /** @short This signal is emitted then the server sent us an ALERT response code */
    void alertReceived( const QString& message );
    /** @short The network went offline

      This signal is emitted if the network connection went offline for any reason.
    Common reasons are an explicit user action or a network error.
 */
    void networkPolicyOffline();
    /** @short The network access policy got changed to "expensive" */
    void networkPolicyExpensive();
    /** @short The network is available and cheap again */
    void networkPolicyOnline();
    /** @short A connection error has been encountered */
    void connectionError( const QString& message );
    /** @short The server requests the user to authenticate

      The user is expected to file username and password to the QAuthenticator* object.
*/
    void authRequested( QAuthenticator* auth );

    /** @short The authentication attempt has failed

Slots attached to his signal should display an appropriate message to the user and (if applicable) also invalidate
the cached credentials.  The credentials be requested when the model decides to try logging in again via the usual
authRequested() function.
*/
    void authAttemptFailed(const QString &message);

    /** @short The amount of messages in the indicated mailbox might have changed */
    void messageCountPossiblyChanged( const QModelIndex& mailbox );

    /** @short We've succeeded to create the given mailbox */
    void mailboxCreationSucceded( const QString& mailbox );
    /** @short The mailbox creation failed for some reason */
    void mailboxCreationFailed( const QString& mailbox, const QString& message );
    /** @short We've succeeded to delete a mailbox */
    void mailboxDeletionSucceded( const QString& mailbox );
    /** @short Mailbox deletion failed */
    void mailboxDeletionFailed( const QString& mailbox, const QString& message );

    /** @short Inform the GUI about the progress of a connection */
    void connectionStateChanged( QObject* parser, Imap::ConnectionState state ); // got to use fully qualified namespace here

    /** @short An interaction with the remote server is taking place */
    void activityHappening( bool isHappening );

    /** @short The parser has encountered a fatal error */
    void logParserFatalError( uint parser, const QString& exceptionClass, const QString& message, const QByteArray& line, int position );

    void mailboxSyncingProgress( const QModelIndex &mailbox, Imap::Mailbox::MailboxSyncingProgress state );

    void mailboxFirstUnseenMessage( const QModelIndex &maillbox, const QModelIndex &message );

    /** @short Threading has arrived */
    void threadingAvailable( const QModelIndex &mailbox, const QString &algorithm,
                             const QStringList &searchCriteria, const QVector<Imap::Responses::ThreadingNode> &mapping );

    /** @short Failed to obtain threading information */
    void threadingFailed(const QModelIndex &mailbox, const QString &algorithm, const QStringList &searchCriteria);

    void capabilitiesUpdated(const QStringList &capabilities);

    void logged(uint parserId, const Imap::Mailbox::LogMessage &message);

private:
    Model& operator=( const Model& ); // don't implement
    Model( const Model& ); // don't implement


    friend class TreeItem;
    friend class TreeItemMailbox;
    friend class TreeItemMsgList;
    friend class TreeItemMessage;
    friend class TreeItemPart;
    friend class TreeItemModifiedPart; // needs access to createIndex()
    friend class MsgListModel; // needs access to createIndex()
    friend class MailboxModel; // needs access to createIndex()
    friend class ThreadingMsgListModel; // needs access to _taskFactory

    friend class DelayedAskForChildrenOfMailbox; // needs access to _askForChildrenOfMailbox();
    friend class DelayedAskForMessagesInMailbox; // needs access to _askForMessagesInMailbox();
    friend class IdleLauncher;

    friend class ImapTask;
    friend class FetchMsgPartTask;
    friend class UpdateFlagsTask;
    friend class ListChildMailboxesTask;
    friend class NumberOfMessagesTask;
    friend class FetchMsgMetadataTask;
    friend class ExpungeMailboxTask;
    friend class CreateMailboxTask;
    friend class DeleteMailboxTask;
    friend class CopyMoveMessagesTask;
    friend class ObtainSynchronizedMailboxTask;
    friend class KeepMailboxOpenTask;
    friend class OpenConnectionTask;
    friend class GetAnyConnectionTask;
    friend class Fake_ListChildMailboxesTask;
    friend class Fake_OpenConnectionTask;
    friend class NoopTask;
    friend class ThreadTask;
    friend class UnSelectTask;

    friend class TestingTaskFactory; // needs access to _socketFactory

    friend class ::FakeCapabilitiesInjector; // for injecting fake capabilities
    friend class ::ImapModelIdleTest; // needs access to findTaskResponsibleFor() for IDLE testing

    void _askForChildrenOfMailbox( TreeItemMailbox* item );
    void _askForMessagesInMailbox( TreeItemMsgList* item );
    void _askForNumberOfMessages( TreeItemMsgList* item );
    void _askForMsgMetadata( TreeItemMessage* item );
    void _askForMsgPart( TreeItemPart* item, bool onlyFromCache=false );

    void _finalizeList( Parser* parser, TreeItemMailbox* const mailboxPtr );
    void _finalizeIncrementalList( Parser* parser, const QString& parentMailboxName );
    void _finalizeFetchPart( TreeItemMailbox* const mailbox, const uint sequenceNo, const QString &partId );
    void _genericHandleFetch( TreeItemMailbox* mailbox, const Imap::Responses::Fetch* const resp );

    void replaceChildMailboxes( TreeItemMailbox* mailboxPtr, const QList<TreeItem*> mailboxes );
    void updateCapabilities( Parser* parser, const QStringList capabilities );

    TreeItem* translatePtr( const QModelIndex& index ) const;

    void emitMessageCountChanged( TreeItemMailbox* const mailbox );
    /** @short Helper for cleaning the QAuthenticator and informing about auth failure */
    void emitAuthFailed(const QString &message);

    TreeItemMailbox* findMailboxByName( const QString& name ) const;
    TreeItemMailbox* findMailboxByName( const QString& name, const TreeItemMailbox* const root ) const;
    TreeItemMailbox* findParentMailboxByName( const QString& name ) const;
    QList<TreeItemMessage*> findMessagesByUids( const TreeItemMailbox* const mailbox, const QList<uint> &uids );

    static TreeItemMailbox* mailboxForSomeItem( QModelIndex index );

    void saveUidMap( TreeItemMsgList* list );

    /** @short Return a corresponding KeepMailboxOpenTask for a given mailbox */
    KeepMailboxOpenTask* findTaskResponsibleFor( const QModelIndex& mailbox );
    KeepMailboxOpenTask* findTaskResponsibleFor( TreeItemMailbox *mailboxPtr );

    /** @short Find a mailbox which is expected to be common for all passed items

    The @arg items is expected to consists of message parts or messages themselves.
    If they belong to different mailboxes, an exception is thrown.
*/
    QModelIndex findMailboxForItems( const QModelIndexList& items );

    NetworkPolicy networkPolicy() const { return _netPolicy; }
    void setNetworkPolicy( const NetworkPolicy policy );

    /** @short Helper function for changing connection state */
    void changeConnectionState( Parser* parser, ConnectionState state );

    /** @short Try to authenticate the user to the IMAP server */
    CommandHandle performAuthentication( Imap::Parser* ptr );

    /** @short Check if all the parsers are indeed idling, and update the GUI if so */
    void parsersMightBeIdling();

    /** @short Is the reason for killing the parser an expected one? */
    typedef enum {
        PARSER_KILL_EXPECTED, /**< @short Normal operation */
        PARSER_KILL_HARD /**< @short Sudden, unexpected death */
    } ParserKillingMethod;

    /** @short Dispose of the parser in a C++-safe way */
    void killParser(Parser *parser, ParserKillingMethod method=PARSER_KILL_HARD);

    ParserState& accessParser( Parser *parser );

    /** @short Helper for the slotParseError() */
    void broadcastParseError( const uint parser, const QString& exceptionClass, const QString& errorMessage, const QByteArray& line, int position );

    /** @short Remove deleted Tasks from the activeTasks list */
    void removeDeletedTasks( const QList<ImapTask*>& deletedTasks, QList<ImapTask*>& activeTasks );

    QStringList _onlineMessageFetch;

    /** @short Helper for keeping track of user/pass over time

    The reason for using QAuthenticator* instead of non-pointer member is that isNull()
    and operator=() does not really work together and that I got a mysterious segfault when
    trying the operator=(). Oh well...
    */
    QAuthenticator* _authenticator;

    uint lastParserId;

protected slots:
    void responseReceived( Imap::Parser *parser );

    void runReadyTasks();

};

}

}

#endif /* IMAP_MODEL_H */
