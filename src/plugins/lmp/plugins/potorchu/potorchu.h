/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/ifilterplugin.h>

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public ILMPPlugin
				 , public IFilterPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LC::LMP::ILMPPlugin LC::LMP::IFilterPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.Potorchu")

		ILMPProxy_ptr LmpProxy_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		void SetLMPProxy (ILMPProxy_ptr);

		QList<EffectInfo> GetEffects () const;
	};
}
}
}
