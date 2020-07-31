/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "choroid.h"
#include <QIcon>
#include "choroidtab.h"

namespace LC
{
namespace Choroid
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		TabInfo_.TabClass_ = "ChoroidTab";
		TabInfo_.VisibleName_ = "Choroid";
		TabInfo_.Description_ = tr ("Image viewer tab");
		TabInfo_.Icon_ = GetIcon ();
		TabInfo_.Priority_ = 50;
		TabInfo_.Features_ = TFOpenableByRequest;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Choroid";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Choroid";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Image viewer for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t result;
		result << TabInfo_;
		return result;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "ChoroidTab")
		{
			auto t = new ChoroidTab (TabInfo_, Proxy_, this);

			connect (t,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			emit addNewTab ("Choroid", t);
			emit raiseTab (t);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_choroid, LC::Choroid::Plugin);
