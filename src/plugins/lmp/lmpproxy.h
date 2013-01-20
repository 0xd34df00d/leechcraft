/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include <QObject>
#include "interfaces/lmp/ilmpproxy.h"

namespace LeechCraft
{
namespace LMP
{
	class LMPProxy : public QObject
				   , public ILMPProxy
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::LMP::ILMPProxy)
	public:
		LMPProxy ();

		ILocalCollection* GetLocalCollection () const;
		ITagResolver* GetTagResolver () const;
		QString FindAlbumArt (const QString&, bool) const;
		QList<QFileInfo> RecIterateInfo (const QString&, bool) const;
		QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters () const;
		QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters () const;
		QString PerformSubstitutions (QString mask, const MediaInfo& info) const;
	};
}
}
