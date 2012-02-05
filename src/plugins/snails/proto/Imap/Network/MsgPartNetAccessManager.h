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
#ifndef MSGPARTNETACCESSMANAGER_H
#define MSGPARTNETACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QPersistentModelIndex>

class QUrl;

namespace Gui {
class PartWidgetFactory;
}

namespace Imap {

namespace Mailbox {
class Model;
class TreeItem;
class TreeItemMessage;
class TreeItemPart;
}

namespace Network {

/** @short Implement access to the MIME Parts of the current message and optiojnally also to the public Internet */
class MsgPartNetAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    MsgPartNetAccessManager(QObject* parent=0 );
    void setModelMessage(const QModelIndex &_message);
    Imap::Mailbox::TreeItemPart* pathToPart(const QString& path);
    Imap::Mailbox::TreeItemPart* cidToPart(const QByteArray& cid, Mailbox::Model *model, Mailbox::TreeItem* root);
protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData=0);
signals:
    void requestingExternal(const QUrl& url);
public slots:
    void setExternalsEnabled(bool enabled);
private:
    friend class Gui::PartWidgetFactory;
    QPersistentModelIndex message;

    bool _externalsEnabled;

    MsgPartNetAccessManager(const MsgPartNetAccessManager&); // don't implement
    MsgPartNetAccessManager& operator=(const MsgPartNetAccessManager&); // don't implement
};

}
}
#endif // MSGPARTNETACCESSMANAGER_H
