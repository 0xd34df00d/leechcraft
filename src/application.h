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

#ifndef APPLICATION_H
#define APPLICATION_H
#include <memory>
#include <QApplication>
#include <QStringList>
#include <QTranslator>

namespace LeechCraft
{
	class Application : public QApplication
	{
		Q_OBJECT

		QStringList Arguments_;

		enum Errors
		{
			EAlreadyRunning = 1,
			EPaths = 2,
			EHelpRequested = 3
		};

		std::auto_ptr<QTranslator> Translator_;
	public:
		Application (int&, char**);
		const QStringList& Arguments () const;
		static QString GetSocketName ();
		void InitiateRestart ();

		virtual bool notify (QObject*, QEvent*);
	protected:
		virtual void commitData (QSessionManager&);
		virtual void saveState (QSessionManager&);
	private slots:
		void checkStillRunning ();
	private:
		bool IsAlreadyRunning () const;
		void ParseCommandLine ();
		void EnterRestartMode ();
	};
};

#endif

