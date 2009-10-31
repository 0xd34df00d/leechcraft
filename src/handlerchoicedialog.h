/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef HANDLERCHOICEDIALOG_H
#define HANDLERCHOICEDIALOG_H
#include <map>
#include <memory>
#include <QDialog>
#include <QButtonGroup>
#include "ui_handlerchoicedialog.h"

class IInfo;
class IDownload;
class IEntityHandler;

namespace LeechCraft
{
	class HandlerChoiceDialog : public QDialog
	{
		Q_OBJECT

		Ui::HandlerChoiceDialog Ui_;
		std::auto_ptr<QButtonGroup> Buttons_;
		typedef std::map<QString, IDownload*> downloaders_t;
		downloaders_t Downloaders_;
		typedef std::map<QString, IEntityHandler*> handlers_t;
		handlers_t Handlers_;
		mutable QString Suggestion_;
	public:
		HandlerChoiceDialog (const QString&, QWidget* = 0);

		void SetFilenameSuggestion (const QString&);
		bool Add (const IInfo*, IDownload*);
		bool Add (const IInfo*, IEntityHandler*);
		IDownload* GetDownload ();
		IDownload* GetFirstDownload ();
		IEntityHandler* GetEntityHandler ();
		IEntityHandler* GetFirstEntityHandler ();
		QString GetFilename () const;
		int NumChoices () const;
	private:
		QStringList GetPluginSavePaths (const QString&) const;
	private slots:
		void populateLocationsBox ();
	};
};

#endif

