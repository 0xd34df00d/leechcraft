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
#ifndef IMAP_NET_FORBIDDENREPLY_H
#define IMAP_NET_FORBIDDENREPLY_H

#include <QNetworkReply>

namespace Imap {
namespace Network {

/** @short A network reply which indicates that the request got denied by policy */
class ForbiddenReply : public QNetworkReply
{
Q_OBJECT
public:
    ForbiddenReply(QObject* parent);
protected:
    virtual qint64 readData(char* data, qint64 maxSize)
    {
        Q_UNUSED(data); Q_UNUSED(maxSize);
        return -1;
    }
    virtual void abort() {}
private slots:
    void slotFinish();
};

}
}

#endif // IMAP_NET_FORBIDDENREPLY_H
