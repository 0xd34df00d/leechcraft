/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QXmppDataForm.h>

class QXmppDataForm;
class QWidget;
class QFormLayout;

namespace LC
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
		const QXmppDataForm Form_;
		QHash<QXmppDataForm::Field::Type, FieldHandler_ptr> Type2Handler_;
		QString From_;
		XMPPBobManager *BobManager_;
	public:
		explicit FormBuilder (const QXmppDataForm& form, const QString& from = {}, XMPPBobManager* = nullptr);

		QString From () const;
		XMPPBobManager* BobManager () const;

		QWidget* CreateForm (QWidget *parent = nullptr);
		QXmppDataForm GetUpdatedForm () const;

		QString GetUsername () const;
		QString GetPassword () const;
	};
}
}
}
