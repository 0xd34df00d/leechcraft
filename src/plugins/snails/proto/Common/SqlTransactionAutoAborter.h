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

#ifndef COMMON_SQLTRANSACTIONAUTOABORTER_H
#define COMMON_SQLTRANSACTIONAUTOABORTER_H

#include <QSqlDatabase>

namespace Common {
    /** @short An auto-commiter

      A utility class using the RAII idiom -- when its instance goes out of scope,
      it aborts the current transaction
*/
    class SqlTransactionAutoAborter {
    public:
        SqlTransactionAutoAborter( QSqlDatabase* db ): _db(db), _commited(false)
        {
            _db->transaction();
        }
        ~SqlTransactionAutoAborter()
        {
            if ( ! _commited )
                _db->rollback();
        }
        bool commit()
        {
            _commited = _db->commit();
            return _commited;
        }
    private:
        QSqlDatabase* _db;
        bool _commited;
    };
}

#endif // COMMON_SQLTRANSACTIONAUTOABORTER_H
