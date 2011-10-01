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

#ifndef DCPLUSPLUS_DCPP_TASK_H
#define DCPLUSPLUS_DCPP_TASK_H

#include "CriticalSection.h"

namespace dcpp {

struct Task {
    virtual ~Task() { };
};
struct StringTask : public Task {
    StringTask(const string& str_) : str(str_) { }
    string str;
};

class TaskQueue {
public:
    typedef pair<uint8_t, Task*> Pair;
    typedef vector<Pair> List;
    typedef List::const_iterator Iter;

    TaskQueue() {
    }

    ~TaskQueue() {
        clear();
    }

    void add(uint8_t type, Task* data) { Lock l(cs); tasks.push_back(make_pair(type, data)); }
    void get(List& list) { Lock l(cs); swap(tasks, list); }
    void clear() {
        List tmp;
        get(tmp);
        for(Iter i = tmp.begin(); i != tmp.end(); ++i) {
            delete i->second;
        }
    }
private:

    TaskQueue(const TaskQueue&);
    TaskQueue& operator=(const TaskQueue&);

    CriticalSection cs;
    List tasks;
};

} // namespace dcpp

#endif
