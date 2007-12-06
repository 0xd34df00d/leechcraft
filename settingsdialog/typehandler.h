#ifndef TYPEHANDLER_H
#define TYPEHANDLER_H
#include "typelist.h"

class QVariant;
class Converter;
class SettingsItemInfo;

class TypeHandler
{
	QList<Converter*> Converters_;
public:
	TypeHandler (QObject *parent);
	virtual ~TypeHandler ();
	QWidget* operator() (const QVariant&, const SettingsItemInfo&, const QString&, QObject*);
	bool MakeLabel (const QVariant&, const SettingsItemInfo&) const;
	void UpdateSettings ();
};

#endif

