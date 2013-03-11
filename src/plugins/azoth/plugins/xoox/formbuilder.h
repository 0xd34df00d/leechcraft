/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_FORMBUILDER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_FORMBUILDER_H
#include <memory>
#include <QXmppDataForm.h>

class QXmppDataForm;
class QWidget;
class QFormLayout;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class FieldHandler;
	typedef std::shared_ptr<FieldHandler> FieldHandler_ptr;
	
	class XMPPBobManager;

	class FormBuilder
	{
		QXmppDataForm Form_;
		QHash<QXmppDataForm::Field::Type, FieldHandler_ptr> Type2Handler_;
		QString From_;
		XMPPBobManager *BobManager_;
	public:
		FormBuilder (const QString& = QString (), XMPPBobManager* = 0);

		QString From () const;
		XMPPBobManager* BobManager () const;

		QWidget* CreateForm (const QXmppDataForm&, QWidget* = 0);
		QXmppDataForm GetForm ();

		QString GetSavedUsername () const;
		QString GetSavedPass () const;
	};
}
}
}

#endif
