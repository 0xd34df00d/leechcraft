#include "addfeed.h"

AddFeed::AddFeed (QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
}

QString AddFeed::GetURL () const
{
    return URL_->text ();
}

QStringList AddFeed::GetTags () const
{
    return Tags_->text ().split (' ', QString::SkipEmptyParts);
}

