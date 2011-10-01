/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DCPLUSPLUS_DCPP_FINISHED_MANAGER_LISTENER_H
#define DCPLUSPLUS_DCPP_FINISHED_MANAGER_LISTENER_H

#include "forward.h"

namespace dcpp {

class FinishedManagerListener {
public:
    virtual ~FinishedManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> AddedFile;
    typedef X<1> AddedUser;
    typedef X<2> UpdatedFile;
    typedef X<3> UpdatedUser;
    typedef X<4> RemovedFile;
    typedef X<5> RemovedUser;
    typedef X<6> RemovedAll;

    virtual void on(AddedFile, bool, const string&, const FinishedFileItemPtr&) throw() { }
        virtual void on(AddedUser, bool, const HintedUser&, const FinishedUserItemPtr&) throw() { }
    virtual void on(UpdatedFile, bool, const string&, const FinishedFileItemPtr&) throw() { }
        virtual void on(UpdatedUser, bool, const HintedUser&) throw() { }
    virtual void on(RemovedFile, bool, const string&) throw() { }
        virtual void on(RemovedUser, bool, const HintedUser&) throw() { }
    virtual void on(RemovedAll, bool) throw() { }
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_FINISHED_MANAGER_LISTENER_H)
