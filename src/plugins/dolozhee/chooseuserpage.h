/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include <interfaces/core/icoreproxy.h>
#include "ui_chooseuserpage.h"

namespace LC
{
struct Entity;

namespace Dolozhee
{
	class ChooseUserPage : public QWizardPage
	{
		Q_OBJECT

		Ui::ChooseUserPage Ui_;
		const ICoreProxy_ptr Proxy_;
	public:
		enum class User
		{
			Anonymous,
			New,
			Existing
		};

		explicit ChooseUserPage (const ICoreProxy_ptr&, QWidget* = nullptr);

		void initializePage () override;
		int nextId () const override;
		bool isComplete () const override;

		User GetUser () const;
		QString GetLogin () const;
		QString GetPassword () const;
		QString GetEmail () const;
		QString GetFirstName () const;
		QString GetLastName () const;
	private:
		QString GetPassKey () const;
		void SaveCredentials ();
	};
}
}
