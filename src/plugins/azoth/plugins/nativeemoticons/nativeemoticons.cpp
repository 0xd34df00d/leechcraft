/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "nativeemoticons.h"
#include <QIcon>
#include "nativeemoticonssource.h"
#include "kopeteemoticonssource.h"
#include "psiplusemoticonssource.h"

namespace LeechCraft
{
namespace Azoth
{
namespace NativeEmoticons
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		ResourceSources_ << new NativeEmoticonsSource ()
				<< new KopeteEmoticonsSource ()
				<< new PsiPlusEmoticonsSource ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.NativeEmoticons";
	}

	QString Plugin::GetName () const
	{
		return "Azoth NativeEmoticons";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for native Azoth emoticons packs");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin";
		return result;
	}

	QList<QObject*> Plugin::GetResourceSources () const
	{
		return ResourceSources_;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_nativeemoticons, LeechCraft::Azoth::NativeEmoticons::Plugin);
