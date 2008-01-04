#ifndef DATATYPES_H
#define DATATYPES_H
#include <QMetaType>
#include <QStringList>
#include <QPair>

typedef QPair<QStringList, int> PairedStringList;
Q_DECLARE_METATYPE (PairedStringList);

typedef QPair<int, int> IntRange;
Q_DECLARE_METATYPE (IntRange);

QDataStream& operator<< (QDataStream&, const IntRange&);
QDataStream& operator>> (QDataStream&, IntRange&);

#endif

