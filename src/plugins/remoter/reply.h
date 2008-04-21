#ifndef REPLY_H
#define REPLY_H

enum State
{
    StateOK = 200
    , StateFound = 302
    , StateNotFound = 404
};

struct Reply
{
    State State_;
    QString Data_;
    QString Type_;
    QString RedirectTo_;
};

#endif

