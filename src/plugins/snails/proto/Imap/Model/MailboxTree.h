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

#ifndef IMAP_MAILBOXTREE_H
#define IMAP_MAILBOXTREE_H

#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QString>
#include "../Parser/Response.h"
#include "../Parser/Message.h"
#include "MailboxMetadata.h"

namespace Imap {

namespace Mailbox {

class Model;
class MailboxModel;
class KeepMailboxOpenTask;

class TreeItem {
    friend class Model; // for _loading and _fetched
    TreeItem(const TreeItem&); // don't implement
    void operator=( const TreeItem& ); // don't implement
    friend class DeleteMailboxTask; // for direct access to _children
    friend class ObtainSynchronizedMailboxTask;
    friend class KeepMailboxOpenTask; // for direct access to _children

protected:
    /** @short Availability of an item */
    enum FetchingState {
        NONE, /**< @short No attempt to download an item has been made yet */
        UNAVAILABLE, /**< @short Item isn't cached and remote requests are disabled */
        LOADING, /**< @short Download of an item is already scheduled */
        DONE /**< @short Item is available right now */
    };

public:
    typedef enum {
        /** @short Full body of an e-mail stored on the IMAP server

          This one really makes sense on a TreeItemMessage and TreeItemPart, and
          are used
*/
        /** @short The HEADER fetch modifier for the current item */
        OFFSET_HEADER=1,
        /** @short The TEXT fetch modifier for the current item */
        OFFSET_TEXT=2,
        /** @short The MIME fetch modifier for individual message parts

          In constrast to OFFSET_HEADER and OFFSET_TEXT, this one applies
          only to TreeItemPart, simply because using the MIME modiifer on
          a top-level message is not allowed as per RFC 3501.
*/
        OFFSET_MIME=3
    } PartModifier;

protected:
    TreeItem* _parent;
    QList<TreeItem*> _children;
    FetchingState _fetchStatus;
public:
    TreeItem( TreeItem* parent );
    TreeItem* parent() const { return _parent; }
    virtual int row() const;

    virtual ~TreeItem();
    virtual unsigned int childrenCount( Model* const model );
    virtual TreeItem* child( const int offset, Model* const model );
    virtual QList<TreeItem*> setChildren( const QList<TreeItem*> items );
    virtual void fetch( Model* const model ) = 0;
    virtual unsigned int rowCount( Model* const model ) = 0;
    virtual unsigned int columnCount();
    virtual QVariant data( Model* const model, int role ) = 0;
    virtual bool hasChildren( Model* const model ) = 0;
    virtual bool fetched() const { return _fetchStatus == DONE; }
    virtual bool loading() const { return _fetchStatus == LOADING; }
    virtual bool isUnavailable( Model* const model ) const;
    virtual TreeItem* specialColumnPtr( int row, int column ) const;
    virtual QModelIndex toIndex(Model *const model) const;
};

class TreeItemPart;
class TreeItemMessage;

class TreeItemMailbox: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    MailboxMetadata _metadata;
    friend class Model; // needs access to maintianingTask
    friend class MailboxModel;
    friend class KeepMailboxOpenTask; // needs access to maintainingTask
    static QLatin1String _noInferiors;
    static QLatin1String _hasNoChildren;
    static QLatin1String _hasChildren;
public:
    TreeItemMailbox( TreeItem* parent );
    TreeItemMailbox( TreeItem* parent, Responses::List );

    static TreeItemMailbox* fromMetadata( TreeItem* parent, const MailboxMetadata& metadata );

    virtual QList<TreeItem*> setChildren( const QList<TreeItem*> items );
    virtual void fetch( Model* const model );
    virtual unsigned int rowCount( Model* const model );
    virtual QVariant data( Model* const model, int role );
    virtual bool hasChildren( Model* const model );
    virtual TreeItem* child( const int offset, Model* const model );

    SyncState syncState;

    /** @short Returns true if this mailbox has child mailboxes

This function might access the network if the answer can't be decided, for example on basis of mailbox flags.
*/
    bool hasChildMailboxes( Model* const model );
    /** @short Return true if the mailbox is already known to not have any child mailboxes

No network activity will be caused. If the answer is not known for sure, we return false (meaning "don't know").
*/
    bool hasNoChildMaliboxesAlreadyKnown();

    QString mailbox() const { return _metadata.mailbox; }
    QString separator() const { return _metadata.separator; }
    const MailboxMetadata& mailboxMetadata() const { return _metadata; }
    /** @short Update internal tree with the results of a FETCH response

      If \a changedPart is not null, it will be updated to point to the message
      part whose content got fetched.
    */
    void handleFetchResponse( Model* const model,
                              const Responses::Fetch& response,
                              QList<TreeItemPart*> &changedParts,
                              TreeItemMessage* &changedMessage );
    void handleFetchWhileSyncing( Model* const model, const Responses::Fetch& response );
    void rescanForChildMailboxes( Model* const model );
    void handleExpunge( Model* const model, const Responses::NumberResponse& resp );
    bool isSelectable() const;
private:
    TreeItemPart* partIdToPtr( Model* model, TreeItemMessage* message, const QString& msgId );

    /** @short ImapTask which is currently responsible for well-being of this mailbox */
    QPointer<KeepMailboxOpenTask> maintainingTask;
};

class TreeItemMsgList: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    friend class TreeItemMailbox;
    friend class TreeItemMessage; // for maintaining the _unreadMessageCount
    friend class Model;
    friend class ObtainSynchronizedMailboxTask;
    friend class KeepMailboxOpenTask;
    FetchingState _numberFetchingStatus;
    int _totalMessageCount;
    int _unreadMessageCount;
    int _recentMessageCount;
public:
    TreeItemMsgList( TreeItem* parent );

    virtual void fetch( Model* const model );
    virtual unsigned int rowCount( Model* const model );
    virtual QVariant data( Model* const model, int role );
    virtual bool hasChildren( Model* const model );

    int totalMessageCount( Model* const model );
    int unreadMessageCount( Model* const model );
    int recentMessageCount( Model* const model );
    void fetchNumbers( Model* const model );
    void recalcVariousMessageCounts(Model *model);
    bool numbersFetched() const;
};

class TreeItemMessage: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    friend class TreeItemMailbox;
    friend class TreeItemMsgList;
    friend class Model;
    friend class ObtainSynchronizedMailboxTask; // needs access to _offset
    friend class KeepMailboxOpenTask; // needs access to _offset
    Message::Envelope _envelope;
    uint _size;
    uint _uid;
    QStringList _flags;
    bool _flagsHandled;
    int _offset;
    TreeItemPart* _partHeader;
    TreeItemPart* _partText;
    /** @short Set FLAGS and maintain the unread message counter */
    void setFlags(TreeItemMsgList *list, const QStringList &flags);
public:
    TreeItemMessage( TreeItem* parent );
    ~TreeItemMessage();

    virtual int row() const;
    virtual void fetch( Model* const model );
    virtual unsigned int rowCount( Model* const model );
    virtual unsigned int columnCount();
    virtual QVariant data( Model* const model, int role );
    virtual bool hasChildren( Model* const model ) { Q_UNUSED( model ); return true; }
    Message::Envelope envelope( Model* const model );
    uint size( Model* const model );
    bool isMarkedAsDeleted() const;
    bool isMarkedAsRead() const;
    bool isMarkedAsReplied() const;
    bool isMarkedAsForwarded() const;
    bool isMarkedAsRecent() const;
    uint uid() const;
    virtual TreeItem* specialColumnPtr( int row, int column ) const;
};

class TreeItemPart: public TreeItem {
    void operator=( const TreeItem& ); // don't implement
    friend class TreeItemMailbox; // needs access to _data
    friend class Model; // dtto
    QString _mimeType;
    QString _charset;
    QByteArray _encoding;
    QByteArray _data;
    QByteArray _bodyFldId;
    QByteArray _bodyDisposition;
    QString _fileName;
    uint _octets;
    TreeItemPart* _partHeader;
    TreeItemPart* _partText;
    TreeItemPart* _partMime;
public:
    TreeItemPart( TreeItem* parent, const QString& mimeType );
    ~TreeItemPart();

    virtual unsigned int childrenCount( Model* const model );
    virtual TreeItem* child( const int offset, Model* const model );
    virtual QList<TreeItem*> setChildren( const QList<TreeItem*> items );

    virtual void fetchFromCache( Model* const model );
    virtual void fetch( Model* const model );
    virtual unsigned int rowCount( Model* const model );
    virtual unsigned int columnCount();
    virtual QVariant data( Model* const model, int role );
    virtual bool hasChildren( Model* const model );

    virtual QString partId() const;
    virtual QString partIdForFetch() const;
    virtual QString pathToPart() const;
    TreeItemMessage* message() const;

    /** @short Provide access to the internal buffer holding data

        It is safe to access the obtained pointer as long as this object is not
        deleted. This function violates the classic concept of object
        encapsulation, but is really useful for the implementation of
        Imap::Network::MsgPartNetworkReply.
     */
    QByteArray* dataPtr();
    QString mimeType() const { return _mimeType; }
    QString charset() const { return _charset; }
    void setCharset( const QString& ch ) { _charset = ch; }
    void setEncoding( const QByteArray& encoding ) { _encoding = encoding; }
    QByteArray encoding() const { return _encoding; }
    void setBodyFldId( const QByteArray& id ) { _bodyFldId = id; }
    QByteArray bodyFldId() const { return _bodyFldId; }
    void setBodyDisposition( const QByteArray& disposition ) { _bodyDisposition = disposition; }
    QByteArray bodyDisposition() const { return _bodyDisposition; }
    void setFileName( const QString& name ) { _fileName = name; }
    QString fileName() const { return _fileName; }
    void setOctets( const uint size ) { _octets = size; }
    /** @short Return the downloadable size of the message part */
    uint octets() const { return _octets; }
    virtual TreeItem* specialColumnPtr( int row, int column ) const;

    void silentlyReleaseMemoryRecursive();
protected:
    virtual bool isTopLevelMultiPart() const;
    TreeItemPart(TreeItem *parent);
};

/** @short A message part with a modifier

This item hanldes fetching of message parts with an attached modifier (like TEXT, HEADER or MIME).
*/
class TreeItemModifiedPart: public TreeItemPart {
    PartModifier _modifier;
public:
    TreeItemModifiedPart( TreeItem* parent, const PartModifier kind );
    virtual int row() const;
    virtual unsigned int columnCount();
    virtual QString partId() const;
    virtual QString pathToPart() const;
    virtual TreeItem* specialColumnPtr( int row, int column ) const;
    PartModifier kind() const;
    virtual QModelIndex toIndex(Model *const model) const;
protected:
    virtual bool isTopLevelMultiPart() const;
private:
    QString modifierToString() const;
};

}

}

#endif // IMAP_MAILBOXTREE_H
