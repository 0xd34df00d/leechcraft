/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QPair>
#include <QList>
#include <QRegExp>

class QWidget;
class QString;
class QAction;

namespace LeechCraft
{
	enum class ContentType
	{
		HTML,
		PlainText
	};
	
	enum class EditorAction
	{
		Find,
		Replace
	};
}

class IEditorWidget
{
public:
	virtual ~IEditorWidget () {}

	virtual QString GetContents (LeechCraft::ContentType) const = 0;

	virtual void SetContents (const QString& contents, LeechCraft::ContentType) = 0;
	
	virtual QAction* GetEditorAction (LeechCraft::EditorAction) = 0;

	virtual void AppendAction (QAction*) = 0;
	virtual void RemoveAction (QAction*) = 0;
};

class IAdvancedHTMLEditor
{
public:
	virtual ~IAdvancedHTMLEditor () {}

	typedef QPair<QRegExp, QString> Replacement_t;
	typedef QList<Replacement_t> Replacements_t;

	virtual void InsertHTML (const QString& html) = 0;

	virtual void SetTagsMappings (const Replacements_t& rich2html, const Replacements_t& html2rich) = 0;

	virtual void ExecJS (const QString& js) = 0;
};

class ITextEditor
{
public:
	virtual ~ITextEditor () {}

	virtual bool SupportsEditor (LeechCraft::ContentType) const = 0;

	virtual QWidget* GetTextEditor (LeechCraft::ContentType) = 0;
};

Q_DECLARE_INTERFACE (IEditorWidget, "org.Deviant.LeechCraft.IEditorWidget/1.0");
Q_DECLARE_INTERFACE (IAdvancedHTMLEditor, "org.Deviant.LeechCraft.IAdvancedHTMLEditor/1.0");
Q_DECLARE_INTERFACE (ITextEditor, "org.Deviant.LeechCraft.ITextEditor/1.0");
