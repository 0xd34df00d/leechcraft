/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Andrey Batyiev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagemediawidget.h"
#include <QUrl>
#include <QDebug>
#include "xeps/xmppbobmanager.h"
#include "xeps/xmppbobiq.h"

namespace LC::Azoth::Xoox
{
	ImageMediaWidget::ImageMediaWidget (const QUrl& uri, XMPPBobManager *manager, const QString& from, QWidget *parent)
	: QLabel (parent)
	{
		QByteArray data;
		QString cid;
		if (uri.scheme () == "cid")
		{
			cid = uri.path ();
			data = manager->Take (from, cid);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unhandled uri:"
					<< uri;

		if (!data.isNull ())
			setPixmap (QPixmap::fromImage (QImage::fromData (data)));
		else if (!cid.isEmpty ())
		{
			connect (manager,
					&XMPPBobManager::bobReceived,
					this,
					[this, cid] (const XMPPBobIq& bob)
					{
						if (bob.GetCid () == cid)
							setPixmap (QPixmap::fromImage (QImage::fromData (bob.GetData ())));
					});
			manager->RequestBob (from, cid);
		}
	}
}
