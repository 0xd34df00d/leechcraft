#ifndef JOBLISTITEM_H
#define JOBLISTITEM_H
#include <QTreeWidgetItem>

class JobListItem : public QTreeWidgetItem
{
 unsigned int ID_;

 enum { eimytype = Type };
public:
 JobListItem ();
 JobListItem (const QTreeWidgetItem& other);

 void SetID (unsigned int);
 unsigned int GetID () const;
};

#endif

