/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QApplication>
#include <QStringList>

namespace LC::CL
{
	struct Args;
}

namespace LC
{
	class SplashScreen;

	/** Manages the main application-level behavior of LeechCraft like
	 * single-instance feature, communication with Session Manager,
	 * restarting, parsing command line and such.
	 */
	class Application : public QApplication
	{
		Q_OBJECT

		QStringList Arguments_;

		const QString DefaultSystemStyleName_;
		QString PreviousLangName_;

		std::unique_ptr<CL::Args> CLArgs_;

		SplashScreen *Splash_ = nullptr;
	public:
		enum Errors
		{
			EAlreadyRunning = 1,
			EPaths = 2,
			EHelpRequested = 3,
			EGeneralSocketError = 4,
			EVersionRequested = 5
		};

		/** Constructs the Application, parses the command line,
		 * installs Qt-wide translations, performs some basic checks
		 * and registers commonly used meta types.
		 *
		 * @param[in] argc argc from main().
		 * @param[in] argv argcvfrom main().
		 */
		Application (int& argc, char **argv);

		~Application () override;

		/** Returns the cached copy of QCoreApplication::arguments().
		 * Provided for performance reasons, as Qt docs say that calling
		 * the original function is slow.
		 *
		 * @return Cached copy of QCoreApplication::arguments().
		 */
		const QStringList& Arguments () const;

		const CL::Args& GetParsedArguments () const;

		/** Returns the local socket name based on the user name/id and
		 * such things.
		 *
		 * @return String with the socket name.
		 */
		static QString GetSocketName ();

		/** Returns the splash screen during LeechCraft startup.
		 *
		 * After finishInit() is invoked, the return value of this
		 * function is undefined.
		 */
		SplashScreen* GetSplashScreen () const;

		/** Performs restart: starts a detached copy with '-restart'
		 * switch and calls qApp->quit().
		 */
		void InitiateRestart ();

		void Quit ();

		/** Checks whether another instance of LeechCraft is running.
		 */
		bool IsAlreadyRunning () const;

		/** Overloaded QApplication::notify() provided to catch exceptions
		 * in slots.
		 */
		bool notify (QObject*, QEvent*) override;
	private:
		void InstallMsgHandlers ();

		void InitPluginsIconset ();

		/** Enter the restart mode. This is called in case leechcraft is
		 * started with the '-restart' option.
		 */
		void EnterRestartMode ();

		void CheckStartupPass ();
		void InitSettings ();

		void InitSessionManager ();
	private slots:
		void finishInit ();

		void handleAppStyle ();
		void handleLanguage ();

		/** Checks whether another copy of LeechCraft is still running
		 * via a call to IsAlreadyRunning(), and if it isn't, starts a
		 * new leechcraft process with the corresponding arguments.
		 */
		void checkStillRunning ();
	};
};
