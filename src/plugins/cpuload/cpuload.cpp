/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cpuload.h"
#include <QIcon>
#include <QAbstractItemModel>
#include <util/util.h>

#ifdef Q_OS_LINUX
#include "linuxbackend.h"
#elif defined (Q_OS_FREEBSD)
#include "freebsdbackend.h"
#elif defined (Q_OS_MAC)
#include "macbackend.h"
#endif

#include "backendproxy.h"

namespace LC
{
namespace CpuLoad
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("cpuload");

#ifdef Q_OS_LINUX
		auto backend = new LinuxBackend;
#elif defined (Q_OS_FREEBSD)
		auto backend = new FreeBSDBackend;
#elif defined (Q_OS_MAC)
		auto backend = new MacBackend;
#else
		Backend *backend = nullptr;
#endif

		if (!backend)
			return;

		CpuQuark_ = std::make_shared<QuarkComponent> ("cpuload", "CpuLoadQuark.qml");

		auto backendProxy = new BackendProxy { backend };
		CpuQuark_->DynamicProps_.append ({ "CpuLoad_proxy", backendProxy });
		CpuQuark_->DynamicProps_.append ({ "CpuLoad_model", backendProxy->GetModel () });
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.CpuLoad";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "CPU Load";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Quark for monitoring CPU load.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		if (CpuQuark_)
			return { CpuQuark_ };
		else
			return {};
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_cpuload, LC::CpuLoad::Plugin);
