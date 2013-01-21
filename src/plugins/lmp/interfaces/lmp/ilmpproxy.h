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

#pragma once

#include <functional>
#include <memory>
#include <QtPlugin>
#include <QFileInfo>
#include <QMap>
#include <QVariant>

class QPixmap;

namespace LeechCraft
{
namespace LMP
{
	class ILocalCollection;
	class ITagResolver;
	struct MediaInfo;

	class ILMPProxy
	{
	public:
		virtual ~ILMPProxy () {}

		virtual ILocalCollection* GetLocalCollection () const = 0;

		virtual ITagResolver* GetTagResolver () const = 0;

		virtual QString FindAlbumArt (const QString& near, bool includeCollection = true) const = 0;

		virtual QList<QFileInfo> RecIterateInfo (const QString& dirPath, bool followSymlinks = false) const = 0;

		virtual QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters () const = 0;

		virtual QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters () const = 0;

		virtual QString PerformSubstitutions (QString mask, const MediaInfo& info) const = 0;
	};

	typedef std::shared_ptr<ILMPProxy> ILMPProxy_Ptr;
}
}

Q_DECLARE_INTERFACE (LeechCraft::LMP::ILMPProxy, "org.LeechCraft.LMP.ILMPProxy/1.0");
