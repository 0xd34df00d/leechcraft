/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_QROSP_WRAPPERS_HOOKPROXYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_HOOKPROXYWRAPPER_H
#include <QObject>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			class HookProxyWrapper : public QObject
			{
				Q_OBJECT
				Q_PROPERTY (QVariant ReturnValue READ GetReturnValue WRITE SetReturnValue);

				IHookProxy_ptr Proxy_;
			public:
				HookProxyWrapper (IHookProxy_ptr);
			public slots:
				void CancelDefault ();
				const QVariant& GetReturnValue () const;
				void SetReturnValue (const QVariant&);
			};
		};
	};
};

#endif
