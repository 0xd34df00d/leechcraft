#include <QtDebug>
#include "typehandler.h"
#include "settingsiteminfo.h"
#include "../exceptions/logic.h"
#include "converters.h"

TypeHandler::TypeHandler (QObject *parent)
{
    Converters_.append (new QStringConverter (parent));
    Converters_.append (new IntConverter (parent));
    Converters_.append (new UIntConverter (parent));
    Converters_.append (new BoolConverter (parent));
    Converters_.append (new QStringListConverter (parent));
    Converters_.append (new PairedStringListConverter (parent));
    Converters_.append (new IntRangeConverter (parent));
}

TypeHandler::~TypeHandler ()
{
    while (!Converters_.isEmpty ())
        delete Converters_.takeLast ();
}

QWidget* TypeHandler::operator() (const QVariant& var, const SettingsItemInfo& sii, const QString& propName, QObject* owner)
{
    for (int i = 0; i < Converters_.size (); ++i)
        if (var.userType () == Converters_ [i]->GetType ())
            return Converters_ [i]->Convert (var, sii, propName, owner);
    return 0;
}

bool TypeHandler::MakeLabel (const QVariant& var, const SettingsItemInfo& sii) const
{
    for (int i = 0; i < Converters_.size (); ++i)
        if (var.type () == Converters_ [i]->GetType ())
            return Converters_ [i]->MakeLabel ();
    return true;
}

void TypeHandler::UpdateSettings ()
{
    for (int i = 0; i < Converters_.size (); ++i)
        Converters_ [i]->UpdateSettings ();
}

