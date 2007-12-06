#ifndef DATATYPES_H
#define DATATYPES_H
#include <QMetaType>
#include <QStringList>
#include <QPair>

typedef QPair<QStringList, int> PairedStringList;
Q_DECLARE_METATYPE (PairedStringList);

#endif

