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
#include <QDebug>
#include <QTimer>
#include "DelayedPopulation.h"
#include "MailboxTree.h"
#include "Model.h"

namespace Imap {

namespace Mailbox {

DelayedAskForChildrenOfMailbox::DelayedAskForChildrenOfMailbox(Model *model, const QModelIndex &mailbox):
    QObject(model), m_model(model), m_mailbox(mailbox),
    // There's a catch with the meaning of "invalid index", see the documentation in the header
    m_topLevel(!mailbox.isValid())
{
    QTimer::singleShot(0, this, SLOT(askNow()));
}

void DelayedAskForChildrenOfMailbox::askNow()
{
    Q_ASSERT(m_model);
    TreeItemMailbox *mailboxPtr = 0;
    if ( m_topLevel ) {
        // We're asked for the root, and we can find that
        mailboxPtr = m_model->_mailboxes;
    } else {
        // The index was previously valid, so let's check if it remains so
        if ( ! m_mailbox.isValid() ) {
            qDebug() << "DelayedAskForChildrenOfMailbox: lost mailbox";
            deleteLater();
            return;
        }
        mailboxPtr = dynamic_cast<TreeItemMailbox*>(static_cast<TreeItem*>(m_mailbox.internalPointer()));
    }
    Q_ASSERT(mailboxPtr);
    m_model->_askForChildrenOfMailbox(mailboxPtr);
    // We're responsible for cleaning up
    deleteLater();
}


DelayedAskForMessagesInMailbox::DelayedAskForMessagesInMailbox(Model *model, const QModelIndex &list):
    QObject(model), m_model(model), m_list(list)
{
    QTimer::singleShot(0, this, SLOT(askNow()));
}

void DelayedAskForMessagesInMailbox::askNow()
{
    Q_ASSERT(m_model);
    if ( ! m_list.isValid() ) {
        qDebug() << "DelayedAskForMessages: lost mailbox";
        deleteLater();
        return;
    }
    TreeItemMsgList *list = dynamic_cast<TreeItemMsgList*>(static_cast<TreeItem*>(m_list.internalPointer()));
    Q_ASSERT(list);
    m_model->_askForMessagesInMailbox(list);
    // We're responsible for cleaning up
    deleteLater();
}

}

}
