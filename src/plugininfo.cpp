/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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

