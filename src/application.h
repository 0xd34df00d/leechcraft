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
	/** Manages the main application-level behavior of LeechCraft like
	 * single-instance feature, communication with Session Manager,
	 * restarting, parsing command line and such.
	 */
	class Application : public QApplication
	{
		Q_OBJECT

		QStringList Arguments_;

		std::auto_ptr<QTranslator> Translator_;
	public:
		enum Errors
		{
			EAlreadyRunning = 1,
			EPaths = 2,
			EHelpRequested = 3,
			EGeneralSocketError = 4
		};

		/** Constructs the Application, parses the command line,
		 * installs Qt-wide translations, performs some basic checks
		 * and registers commonly used meta types.
		 *
		 * @param[in] argc argc from main().
		 * @param[in] argv argcvfrom main().
		 */
		Application (int& argc, char **argv);

		/** Returns the cached copy of QCoreApplication::arguments().
		 * Provided for performance reasons, as Qt docs say that calling
		 * the original function is slow.
		 *
		 * @return Cached copy of QCoreApplication::arguments().
		 */
		const QStringList& Arguments () const;

		/** Returns the local socket name based on the user name/id and
		 * such things.
		 *
		 * @return String with the socket name.
		 */
		static QString GetSocketName ();

		/** Performs restart: starts a detached copy with '-restart'
		 * switch and calls qApp->quit().
		 */
		void InitiateRestart ();

		/** Checks whether another instance of LeechCraft is running.
		 */
		bool IsAlreadyRunning () const;

		/** Overloaded QApplication::notify() provided to catch exceptions
		 * in slots.
		 */
		virtual bool notify (QObject*, QEvent*);
	protected:
		/** Communicates with the sm according to the settings and
		 * command line options.
		 */
		virtual void commitData (QSessionManager& sm);

		/** Communicates with the sm according to the settings and
		 * command line options.
		 */
		virtual void saveState (QSessionManager& sm);
	private slots:
		/** Checks whether another copy of LeechCraft is still running
		 * via a call to IsAlreadyRunning(), and if it isn't, starts a
		 * new leechcraft process with the corresponding arguments.
		 */
		void checkStillRunning ();
	private:
		/** Parses command line and sets corresponding application-wide
		 * options.
		 */
		void ParseCommandLine ();

		/** Enter the restart mode. This is called in case leechcraft is
		 * started with the '-restart' option.
		 */
		void EnterRestartMode ();
	};
};

#endif

