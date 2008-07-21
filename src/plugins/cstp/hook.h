#ifndef HOOK_H
#define HOOK_H
#include <QString>
#include <QPair>
#include <QList>

class Hook
{
public:
	virtual void Act (int, const QString&) const;
	virtual void Act (const QList<QPair<QString, QString> >&) const;
};

#endif

