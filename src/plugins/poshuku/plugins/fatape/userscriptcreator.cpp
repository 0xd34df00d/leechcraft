/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userscriptcreator.h"

#include <QMessageBox>
#include <QFile>
#include <QInputDialog>
#include <QStandardPaths>
#include "createscriptdialog.h"
#include "userscript.h"
#include "fatape.h"

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	UserScriptCreator::UserScriptCreator (Plugin *plugin, QWidget *parent)
	: QObject { parent }
	, Plugin_ { plugin }
	, ParentWidget_ { parent }
	, Dia_ { std::make_shared<CreateScriptDialog> (parent) }
	{
		connect (Dia_.get (),
				SIGNAL (rejected ()),
				this,
				SLOT (deleteLater ()));
		connect (Dia_.get (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));

		Dia_->show ();
	}

	namespace
	{
		std::shared_ptr<QFile> CreateFile (const QString& scriptName, QWidget *parent)
		{
			const auto& temp = QStandardPaths::writableLocation (QStandardPaths::TempLocation);

			auto filename = scriptName;
			while (true)
			{
				const auto& fullPath = temp + '/' + filename + ".user.js";
				auto file = std::make_shared<QFile> (fullPath);
				if (file->open (QIODevice::WriteOnly))
					return file;

				if (QMessageBox::question (parent,
							"LeechCraft",
							UserScriptCreator::tr ("Unable to create file %1: %2. Do you want to try again changing the file name?")
								.arg ("<em>" + fullPath + "</em>")
								.arg (file->errorString ()),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return {};

				filename = QInputDialog::getText (parent,
						"LeechCraft",
						UserScriptCreator::tr ("Enter new filename for the user script %1:")
							.arg ("<em>" + scriptName + "</em>"),
						QLineEdit::Normal,
						filename);
				if (filename.isEmpty ())
					return {};
			}
		}
	}

	QString UserScriptCreator::GenerateFile () const
	{
		const auto& file = CreateFile (Dia_->GetName (), ParentWidget_);
		if (!file)
			return {};

		const QStringList lines
		{
			"// ==UserScript==",
			"// @name           " + Dia_->GetName (),
			"// @namespace      " + Dia_->GetNamespace (),
			"// @description    " + Dia_->GetDescription (),
			"// @author         " + Dia_->GetAuthor (),
			"// ==/UserScript==",
			"(function() {",
			"})();"
		};

		for (const auto& line : lines)
		{
			file->write (line.toUtf8 ());
			file->write ("\n");
		}

		return file->fileName ();
	}

	void UserScriptCreator::handleAccepted ()
	{
		const auto& path = GenerateFile ();
		if (path.isEmpty ())
			return;

		UserScript script { path };
		script.Install ();
		Plugin_->EditScript (Plugin_->AddScriptToManager (script));
	}
}
}
}
