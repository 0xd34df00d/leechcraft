#include <plugininterface/tagscompleter.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "addfeed.h"

using LeechCraft::Util::TagsCompleter;

AddFeed::AddFeed (const QString& url, QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
    new TagsCompleter (Tags_, this);
	Tags_->AddSelector ();

	URL_->setText (url);
}

QString AddFeed::GetURL () const
{
    return URL_->text ().simplified ();
}

QStringList AddFeed::GetTags () const
{
    return Core::Instance ().GetProxy ()->GetTagsManager ()->Split (Tags_->text ());
}

