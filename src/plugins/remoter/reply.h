#ifndef REPLY_H
#define REPLY_H

enum State
{
    StateOK = 200
    , StateUnauthorized = 401
    , StateNotFound = 404
};

struct Reply
{
    State State_;
    QString Data_;
    QString Type_;
};

#endif

