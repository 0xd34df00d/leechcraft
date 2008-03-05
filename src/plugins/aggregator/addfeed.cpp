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
