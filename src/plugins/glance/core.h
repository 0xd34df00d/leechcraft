/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_GLANCE_CORE_H
#define PLUGINS_GLANCE_CORE_H
#include <QObject>
#include <interfaces/iinfo.h>

class QMainWindow;

namespace LC
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
