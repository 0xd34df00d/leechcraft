#include "plugininfo.h"

PluginInfo::PluginInfo (const QString& name,
						const QString& info,
						const QIcon& icon,
						const QString& status,
						const QStringList& provides,
						const QStringList& needs,
						const QStringList& uses,
						bool dm,
						const QStringList& failedDeps)
: Name_ (name)
, Info_ (info)
, Icon_ (icon)
, StatusBarMessage_ (status)
, Provides_ (provides)
, Needs_ (needs)
, Uses_ (uses)
, FailedDeps_ (failedDeps)
, DependenciesMet_ (dm)
{
}

const QString& PluginInfo::GetName () const
{
	return Name_;
}

const QString& PluginInfo::GetInfo () const
{
	return Info_;
}

const QIcon& PluginInfo::GetIcon () const
{
	return Icon_;
}

const QString& PluginInfo::GetStatusbarMessage () const
{
	return StatusBarMessage_;
}

const QStringList& PluginInfo::GetProvides () const
{
	return Provides_;
}

const QStringList& PluginInfo::GetNeeds () const
{
	return Needs_;
}

const QStringList& PluginInfo::GetUses () const
{
	return Uses_;
}

bool PluginInfo::GetDependenciesMet () const
{
	return DependenciesMet_;
}

const QStringList& PluginInfo::GetFailedDeps () const
{
	return FailedDeps_;
}

