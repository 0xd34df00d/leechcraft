/*
    LeechCraft - modular cross-platform feature rich internet client.
    Copyright (C) 2010-2011  Oleg Linkin
    Copyright (C) 2006-2011  Georg Rudoy

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PLUGINS_GLANCE_CORE_H
#define PLUGINS_GLANCE_CORE_H

#include <QObject>
#include <interfaces/iinfo.h>

class QMainWindow;

namespace LeechCraft
{
namespace Plugins
{
namespace Glance
{
	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		Core ();
	public:
		static Core& Instance ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		QMainWindow* GetMainWindow () const;
	};
};
};
};
#endif // PLUGINS_GLANCE_CORE_H
