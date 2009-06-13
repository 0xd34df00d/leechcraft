#include "plugininfo.h"

Main::PluginInfo::PluginInfo (const QString& name,
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

const QString& Main::PluginInfo::GetName () const
{
	return Name_;
}

const QString& Main::PluginInfo::GetInfo () const
{
	return Info_;
}

const QIcon& Main::PluginInfo::GetIcon () const
{
	return Icon_;
}

const QString& Main::PluginInfo::GetStatusbarMessage () const
{
	return StatusBarMessage_;
}

const QStringList& Main::PluginInfo::GetProvides () const
{
	return Provides_;
}

const QStringList& Main::PluginInfo::GetNeeds () const
{
	return Needs_;
}

const QStringList& Main::PluginInfo::GetUses () const
{
	return Uses_;
}

bool Main::PluginInfo::GetDependenciesMet () const
{
	return DependenciesMet_;
}

const QStringList& Main::PluginInfo::GetFailedDeps () const
{
	return FailedDeps_;
}

