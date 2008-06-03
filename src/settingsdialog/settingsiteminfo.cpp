#include "settingsiteminfo.h"

SettingsItemInfo::SettingsItemInfo ()
: Modifiable_ (true)
, Choosable_ (false)
, IntRange_ (-32768, 32768)
, UIntRange_ (0, 65536)
, SpinboxStep_ (1)
, BrowseButton_ (false)
, GroupBoxer_ (false)
{
}

SettingsItemInfo::SettingsItemInfo (const SettingsItemInfo& obj)
: Label_ (obj.Label_)
, Page_ (obj.Page_)
, Group_ (obj.Group_)
, Hint_ (obj.Hint_)
, Modifiable_ (obj.Modifiable_)
, Choosable_ (obj.Choosable_)
, IntRange_ (obj.IntRange_)
, UIntRange_ (obj.UIntRange_)
, SpinboxSuffix_ (obj.SpinboxSuffix_)
, SpinboxStep_ (obj.SpinboxStep_)
, BrowseButton_ (obj.BrowseButton_)
, GroupBoxer_ (obj.GroupBoxer_)
, SubItems_ (obj.SubItems_)
, PageIcon_ (obj.PageIcon_)
{
}

SettingsItemInfo::SettingsItemInfo (const QString& label, const QString& page, const QString& group)
: Label_ (label)
, Page_ (page)
, Group_ (group)
, Modifiable_ (true)
, Choosable_ (false)
, IntRange_ (-32768, 32768)
, UIntRange_ (0, 65536)
, SpinboxStep_ (1)
, BrowseButton_ (false)
, GroupBoxer_ (false)
{
}

SettingsItemInfo& SettingsItemInfo::operator= (const SettingsItemInfo& obj)
{
    Label_ = obj.Label_;
    Page_ = obj.Page_;
    Group_ = obj.Group_;
    Hint_ = obj.Hint_;
    IntRange_ = obj.IntRange_;
    UIntRange_ = obj.UIntRange_;
    SpinboxSuffix_ = obj.SpinboxSuffix_;
    SpinboxStep_ = obj.SpinboxStep_;
    BrowseButton_ = obj.BrowseButton_;
    Modifiable_ = obj.Modifiable_;
    Choosable_ = obj.Choosable_;
    GroupBoxer_ = obj.GroupBoxer_;
    SubItems_ = obj.SubItems_;
    PageIcon_ = obj.PageIcon_;

    return *this;
}

