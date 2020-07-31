/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

class QWidget;

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	class Plugin;
	class CreateScriptDialog;

	class UserScriptCreator : public QObject
	{
		Q_OBJECT

		Plugin * const Plugin_;
		QWidget * const ParentWidget_;
		const std::shared_ptr<CreateScriptDialog> Dia_;
	public:
		UserScriptCreator (Plugin*, QWidget* = nullptr);
	private:
		QString GenerateFile () const;
	private slots:
		void handleAccepted ();
	};
}
}
}
