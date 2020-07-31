/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tpi.h"
#include <QIcon>
#include <QAbstractItemModel>
#include <util/sys/paths.h>
#include <util/gui/util.h>
#include "infomodelmanager.h"

namespace LC
{
namespace TPI
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		ModelMgr_ = new InfoModelManager (proxy);

		auto comp = std::make_shared<QuarkComponent> ("tpi", "TPIQuark.qml");
		comp->DynamicProps_.append ({ "TPI_infoModel", ModelMgr_->GetModel () });
		Components_ << comp;
	}

	void Plugin::SecondInit ()
	{
		ModelMgr_->SecondInit ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TPI";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "TPI";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Task Progress Indicator quark plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return Components_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_tpi, LC::TPI::Plugin);
