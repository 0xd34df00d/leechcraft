#include <plugininterface/tagscompleter.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "addfeed.h"

AddFeed::AddFeed (QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
    TagsCompleter *comp = new TagsCompleter (Tags_, this);
    comp->setModel (Core::Instance ().GetTagsCompletionModel ());
}

QString AddFeed::GetURL () const
{
    return URL_->text ().simplified ();
}

QStringList AddFeed::GetTags () const
{
    return Tags_->text ().split (' ', QString::SkipEmptyParts);
}

