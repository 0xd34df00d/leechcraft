/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_IADDRESSBAR_H
#define PLUGINS_POSHUKU_IADDRESSBAR_H
#include <QtPlugin>
class QAction;
class QToolButton;

namespace LeechCraft
{
namespace Poshuku
{
	class IAddressBar
	{
	public:
		virtual ~IAddressBar () {}

		virtual QObject* GetObject () = 0;

		virtual int ButtonsCount () const = 0;

		virtual QToolButton* AddAction (QAction *action, bool hideOnEmptyUrl = false) = 0;
		virtual QToolButton* InsertAction (QAction *action, int pos = -1, bool hideOnEmptyUrl = false) = 0;

		virtual void RemoveAction (QAction *action) = 0;

		virtual QToolButton* GetButtonFromAction (QAction *action) const = 0;

		virtual void SetVisible (QAction *action, bool visible) = 0;
	protected:
		virtual void actionTriggered (QAction *action, const QString& text) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Poshuku::IAddressBar,
		"org.Deviant.LeechCraft.Poshuku.IAddressBar/1.0");

#endif
