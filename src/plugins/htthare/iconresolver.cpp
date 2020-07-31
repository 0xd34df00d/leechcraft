/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "iconresolver.h"
#include <QImage>
#include <QIcon>
#include <QtDebug>
#include <util/util.h>

namespace LC
{
namespace HttHare
{
	IconResolver::IconResolver (QObject *parent)
	: QObject (parent)
	{
	}

	void IconResolver::resolveMime (QString mimetype, QByteArray& image, int dim)
	{
		mimetype.replace ('/', '-');
		auto icon = QIcon::fromTheme (mimetype);
		if (icon.isNull ())
		{
			mimetype.replace ("x-", "");
			icon = QIcon::fromTheme (mimetype);
		}

		if (icon.isNull ())
			icon = QIcon::fromTheme ("application-octet-stream");

		image = Util::GetAsBase64Src (icon.pixmap (dim, dim).toImage ()).toLatin1 ();
	}
}
}
