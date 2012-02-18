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

#ifndef PLUGINS_VFSCORE_VFSCORE_H
#define PLUGINS_VFSCORE_VFSCORE_H
#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
namespace VFScore
{
	class VFSEngineHandler;

	class Plugin : public QObject
				 , public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		std::shared_ptr<VFSEngineHandler> Handler_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
	};
}
}

#endif
