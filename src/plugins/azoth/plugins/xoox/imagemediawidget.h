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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_IMAGEMEDIAWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_IMAGEMEDIAWIDGET_H
#include <QLabel>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class XMPPBobManager;
	class XMPPBobIq;

	class ImageMediaWidget : public QLabel
	{
		Q_OBJECT

		QString Cid_;
	public:
		ImageMediaWidget (const QPair<QString, QString>&, XMPPBobManager*, const QString&, QWidget* = 0);
	public slots:
		void bobReceived (const XMPPBobIq&);
	};
}
}
}

#endif
