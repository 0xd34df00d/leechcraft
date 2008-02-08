#ifndef SETTINGSITEMINFO_H
#define SETTINGSITEMINFO_H
#include <QString>
#include <QVariant>
#include <QPair>
#include <QIcon>
#include "typelist.h"

struct SettingsItemInfo
{
    QString Label_;
    QString Page_;
    QString Group_;

    QString Hint_;

    bool Modifiable_;
    
    // If QStringList or derived, this represents as QComboBox
    bool Choosable_;

    // If settings item is a number
    QPair<int, int> IntRange_;
    QPair<uint, uint> UIntRange_;
    QString SpinboxSuffix_;
    unsigned int SpinboxStep_;

    // If settings item is QString
    bool BrowseButton_;

    // If settings item is a bool
    bool GroupBoxer_;
    QStringList SubItems_;

    QIcon PageIcon_;

    SettingsItemInfo ();
    SettingsItemInfo (const SettingsItemInfo&);
    SettingsItemInfo (const QString&, const QString&, const QString& group = QString (""));
    SettingsItemInfo& operator= (const SettingsItemInfo&);
};

#endif
