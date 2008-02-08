#include "joblistitem.h"

JobListItem::JobListItem ()
: QTreeWidgetItem (eimytype)
{
}

JobListItem::JobListItem (const QTreeWidgetItem& other)
: QTreeWidgetItem (other)
{
}

void JobListItem::SetID (unsigned int id)
{
   ID_ = id;
}

unsigned int JobListItem::GetID () const
{
   return ID_;
}

