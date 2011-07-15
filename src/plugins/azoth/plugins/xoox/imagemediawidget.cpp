/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Andrey Batyiev
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

#include "imagemediawidget.h"
#include <QXmppBobManager.h>
#include <QXmppBobIq.h>
#include <QDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	ImageMediaWidget::ImageMediaWidget (const QPair<QString, QString>& uri,
			QXmppBobManager *manager, const QString& from, QWidget *parent)
	: QLabel (parent)
	{
		QByteArray data;
		if (uri.second.startsWith ("cid:"))
		{
			Cid_ = uri.second.mid (4);
			data = manager->take (from, Cid_);
		}
		else
		{
			// FIXME
			qWarning () << Q_FUNC_INFO
					<< "unhandled uri:"
					<< uri.second;
		}
		
		if (!data.isNull ())
			setPixmap (QPixmap::fromImage (QImage::fromData (data)));
		else if (!Cid_.isEmpty ())
		{
			connect (manager,
					SIGNAL (bobReceived (const QXmppBobIq&)),
					this,
					SLOT (bobReceived (const QXmppBobIq&)));
			manager->requestBob (from, Cid_);
		}
	}

	void ImageMediaWidget::bobReceived (const QXmppBobIq& bob)
	{
		if (bob.cid () == Cid_)
			setPixmap (QPixmap::fromImage (QImage::fromData (bob.data ())));
	}
}
}
}