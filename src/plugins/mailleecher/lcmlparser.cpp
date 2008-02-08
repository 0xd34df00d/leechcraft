#include "lcmlparser.h"

LCML LCMLParser::Parse (const QByteArray& bytes)
{
    QList<QByteArray> splitted = bytes.trimmed ().split ('\n');
    LCML result;
    result.Address_ = splitted [0];
    result.Login_ = splitted [1];
    for (int i = 2; i < splitted.size (); ++i)
        result.UIDLs_ << splitted [i];

    return result;
}

QByteArray LCMLParser::Create (const LCML& lcml)
{
    QByteArray result;
    result.append (lcml.Address_).append ('\n').append (lcml.Login_);
    for (int i = 0; i < lcml.UIDLs_.size (); ++i)
        result.append ('\n').append (lcml.UIDLs_.at (i));
    return result;
}

