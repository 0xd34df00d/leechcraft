/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <stdexcept>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>
#include <QApplication>
#include "appinfo.h"
#include "crashdialog.h"

namespace CrashProcess = LC::AnHero::CrashProcess;

namespace
{
	namespace bpo = boost::program_options;

	void ShowHelp (const bpo::options_description& desc)
	{
		std::cout << "LeechCraft (https://leechcraft.org)" << std::endl;
		std::cout << std::endl;
		std::cout << "This is the crash handler process and it is not "
				<< "intended to be run manually." << std::endl;
		std::cout << std::endl;
		std::cout << desc << std::endl;
		std::exit (3);
	}

	CrashProcess::AppInfo ParseOptions (int argc, char **argv)
	{
		bpo::options_description desc ("Known options");
		desc.add_options ()
				("signal", bpo::value<int> (), "the signal that triggered the crash handler")
				("pid", bpo::value<uint64_t> ()->required (), "the PID of the crashed process")
				("path", bpo::value<std::string> ()->required (), "the application path of the crashed process")
				("version", bpo::value<std::string> ()->required (), "the LeechCraft version at the moment of the crash")
				("suggest_restart", bpo::value<int> (), "suggest restarting LeechCraft (0 or 1)")
				("cmdline", bpo::value<std::string> (), "the command line LeechCraft was started with")
				("help", "show help message");

		bpo::command_line_parser parser (argc, argv);
		bpo::variables_map vm;
		bpo::store (parser
				.options (desc)
				.allow_unregistered ()
				.run (), vm);

		if (vm.count ("help"))
			ShowHelp (desc);

		try
		{
			bpo::notify (vm);
		}
		catch (const bpo::required_option& e)
		{
			std::cout << "required option missing" << std::endl;
			std::cout << e.what () << std::endl;

			ShowHelp (desc);
		}
		catch (const bpo::error& e)
		{
			std::cout << "invalid options" << std::endl;
			std::cout << e.what () << std::endl;

			ShowHelp (desc);
		}

		return
		{
			.Signal_ = vm ["signal"].as<int> (),
			.PID_ = vm ["pid"].as<uint64_t> (),
			.Path_ = QString::fromUtf8 (vm ["path"].as<std::string> ().c_str ()),
			.Version_ = vm ["version"].as<std::string> ().c_str (),
			.ExecLine_ = vm ["cmdline"].as<std::string> ().c_str (),
			.SuggestRestart_ = vm ["suggest_restart"].as<int> () != 0,
		};
	}
}

int main (int argc, char **argv)
{
	QApplication app (argc, argv);

	const auto& info = ParseOptions (argc, argv);

	new CrashProcess::CrashDialog (info);
	return app.exec ();
}
