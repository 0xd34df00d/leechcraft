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

#ifndef DCPLUSPLUS_DCPP_USER_COMMAND_H
#define DCPLUSPLUS_DCPP_USER_COMMAND_H

#include "Util.h"
#include "Flags.h"

namespace dcpp {

class UserCommand : public Flags {
public:
    typedef vector<UserCommand> List;

    enum {
        TYPE_SEPARATOR,
        TYPE_RAW,
        TYPE_RAW_ONCE,
        TYPE_REMOVE,
                TYPE_CHAT,
                TYPE_CHAT_ONCE,
        TYPE_CLEAR = 255        // In a momentary lapse of reason, 255 was chosen in the nmdc version of usercommand for clearing them all
    };

    enum {
        CONTEXT_HUB = 0x01,
                CONTEXT_USER = 0x02,
        CONTEXT_SEARCH = 0x04,
        CONTEXT_FILELIST = 0x08,
                CONTEXT_MASK = CONTEXT_HUB | CONTEXT_USER | CONTEXT_SEARCH | CONTEXT_FILELIST
    };

    enum {
        FLAG_NOSAVE = 0x01
    };

    UserCommand() : cid(0), type(0), ctx(0) { }
        UserCommand(int aId, int aType, int aCtx, int aFlags, const string& aName, const string& aCommand, const string& aTo, const string& aHub) throw()
                : Flags(aFlags), cid(aId), type(aType), ctx(aCtx), name(aName), command(aCommand), to(aTo), hub(aHub)
        {
                setDisplayName();
        }

    UserCommand(const UserCommand& rhs) : Flags(rhs), cid(rhs.cid), type(rhs.type),
                ctx(rhs.ctx), name(rhs.name), command(rhs.command), to(rhs.to), hub(rhs.hub)
    {
                setDisplayName();
    }

    UserCommand& operator=(const UserCommand& rhs) {
        cid = rhs.cid; type = rhs.type; ctx = rhs.ctx;
                name = rhs.name; command = rhs.command; to = rhs.to; hub = rhs.hub;
        *((Flags*)this) = rhs;
                displayName.clear();
                setDisplayName();
        return *this;
    }

        inline bool isRaw() const {
                return type == TYPE_RAW || type == TYPE_RAW_ONCE;
        }
        inline bool isChat() const {
                return type == TYPE_CHAT || type == TYPE_CHAT_ONCE;
        }
        inline bool once() const {
                return type == TYPE_RAW_ONCE || type == TYPE_CHAT_ONCE;
        }

        static bool adc(const string& h);
        inline bool adc() const { return adc(hub); }

        const StringList& getDisplayName() const;
        void setDisplayName();

    GETSET(int, cid, Id);
    GETSET(int, type, Type);
    GETSET(int, ctx, Ctx);
    GETSET(string, name, Name);
    GETSET(string, command, Command);
        GETSET(string, to, To);
    GETSET(string, hub, Hub);

private:
        StringList displayName;
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_USER_COMMAND_H)
