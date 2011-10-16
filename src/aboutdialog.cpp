/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "aboutdialog.h"
#include "util/sysinfo.h"
#include "interfaces/ihavediaginfo.h"
#include "config.h"
#include "core.h"

namespace LeechCraft
{
	namespace
	{
		struct ContributorInfo
		{
			QString Name_;
			QString Nick_;
			QString JID_;
			QString Email_;
			QStringList Roles_;
			QStringList Years_;

			ContributorInfo (const QString& name, const QString& nick,
					const QString& jid, const QString& email,
					const QStringList& roles, const QList<int>& years)
			: Name_ (name)
			, Nick_ (nick)
			, JID_ (jid)
			, Email_ (email)
			, Roles_ (roles)
			{
				Q_FOREACH (const int year, years)
					Years_ << QString::number (year);
			}

			QString Fmt () const
			{
				QString result = "<strong>";
				if (!Name_.isEmpty ())
					result += Name_;
				if (!Name_.isEmpty () && !Nick_.isEmpty ())
					result += " aka ";
				if (!Nick_.isEmpty ())
					result += Nick_;
				result += "</strong><br/>";

				if (!JID_.isEmpty ())
					result += QString ("JID: <a href=\"xmpp:%1\">%1</a>")
							.arg (JID_);
				if (!JID_.isEmpty () && !Email_.isEmpty ())
					result += "<br />";
				if (!Email_.isEmpty ())
					result += QString ("Email: <a href=\"mailto:%1\">%1</a>")
							.arg (Email_);

				result += "<ul>";
				Q_FOREACH (const QString& r, Roles_)
					result += QString ("<li>%1</li>")
							.arg (r);
				result += "</ul>";

				result += AboutDialog::tr ("Years: %1")
						.arg (Years_.join (", "));

				return result;
			}
		};
	}

	AboutDialog::AboutDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.ProgramName_->setText (QString ("LeechCraft %1")
				.arg (LEECHCRAFT_VERSION));

		QList<ContributorInfo> authors;
		authors << ContributorInfo ("Georg Rudoy", "0xd34df00d",
				"d34df00d@jabber.ru", "0xd34df00d@gmail.com",
				QStringList (tr ("Lead developer and original author.")),
				QList<int> () << 2006 << 2007 << 2008 << 2009 << 2010 << 2011);
		authors << ContributorInfo ("Oleg Linkin", "magog",
				"magog@gentoo.ru", "MaledictusDeMagog@gmail.com",
				QStringList (tr ("Firefox importer in New Life."))
					<< tr ("Poshuku OnlineBookmarks.")
					<< tr ("Azoth Acetamide: IRC support for Azoth.")
					<< tr ("Chrome-style tabs.")
					<< tr ("Various patches."),
				QList<int> () << 2010 << 2011);

		QList<ContributorInfo> contribs;
		contribs << ContributorInfo (QString (), "Akon32",
				QString (), "akon32@rambler.ru",
				QStringList (tr ("SecMan SecureStorage."))
					<< tr ("Various patches."),
				QList<int> () << 2011);
		contribs << ContributorInfo ("Aleksey Frolov", "Aleks Lifey aka atommix",
				QString (), "aleks.lifey@gmail.com",
				QStringList (tr ("Initial PKGBUILDs for Arch Linux.")),
				QList<int> () << 2009);
		contribs << ContributorInfo ("Alexander Batischev", "Minoru",
				QString (), "eual.jp@gmail.com",
				QStringList (tr ("Ukrainian translations.")),
				QList<int> () << 2011);
		contribs << ContributorInfo ("Alia Eolova", "alieola",
				"alieola@jabber.ru", "aeors-team@yandex.ru",
				QStringList (tr ("Spanish translations.")),
				QList<int> () << 2011);
		contribs << ContributorInfo ("Azer Abdullaev", "Like-all",
				QString (), "lik3a11@gmail.com",
				QStringList (tr ("Artwork designer.")),
				QList<int> () << 2011);
		contribs << ContributorInfo ("Elena Belova", "Zereal",
				"elena.zereal@neko.im", "zereal25@gmail.com",
				QStringList (tr ("French translations."))
					<< tr ("Italian translations.")
					<< tr ("Public relations."),
				QList<int> () << 2011);
		contribs << ContributorInfo (QString (), "ForNeVeR",
				"revenrof@jabber.ru", QString (),
				QStringList ("Maintainer for the Microsoft Windows."),
				QList<int> () << 2009 << 2010 << 2011);
		contribs << ContributorInfo (QString (), "Ignotus",
				"nlminhtl@gmail.com", QString (),
				QStringList ("Improvements in Psto.net plugin for Azoth.")
					<< tr ("openSUSE package maintainer")
					<< tr ("Fedora package maintainer"),
				QList<int> () << 2011);
		contribs << ContributorInfo (QString (), "lk4d4",
				QString (), "lk4d4@yander.ru",
				QStringList ("Initial ebuilds for Gentoo Linux."),
				QList<int> () << 2009);
		contribs << ContributorInfo ("Maxim Kirenenko", "part1zan_ aka 0x73571ab",
				"part1zancheg@gmail.com", "part1zancheg@gmail.com",
				QStringList (tr ("Extensive and thorough testing.")),
				QList<int> () << 2010 << 2011);
		contribs << ContributorInfo (QString (), "Miha",
				QString (), "miha@52.ru",
				QStringList ("OpenSUSE package maintainer."),
				QList<int> () << 2009);
		contribs << ContributorInfo (QString (), "nobodyzzz",
				QString (), "nobodyzzz666@gmail.com",
				QStringList ("Juick plugin.")
					<< tr ("FatApe plugin, the GreaseMonkey support layer for Poshuku.")
					<< tr ("Various patches."),
				QList<int> () << 2011);
		contribs << ContributorInfo (QString (), "PanteR",
				"panter_dsd@jabber.ru", "panter.dsd@gmail.com",
				QStringList (tr ("Various patches.")),
				QList<int> () << 2009 << 2010);
		contribs << ContributorInfo (QString (), "Pevzi",
				QString (), "pevzi23@gmail.com",
				QStringList (tr ("Graphical artwork.")),
				QList<int> () << 2009 << 2010);
		contribs << ContributorInfo (QString (), QString::fromUtf8 ("Phóéñíx"),
				"nounou@jabber.ru", QString (),
				QStringList (tr ("Arabic translations.")),
				QList<int> () << 2009 << 2010);
		contribs << ContributorInfo (QString (), "sejros",
				QString (), "home@sejros.mp",
				QStringList (tr ("Esperanto translations")),
				QList<int> () << 2009);
		contribs << ContributorInfo (QString (), "V0id",
				QString (), "getbusy@mail.ru",
				QStringList (tr ("Aggregator fixes and improvements."))
					<< tr ("Various patches.")
					<< tr ("Ukrainian translations."),
				QList<int> () << 2008 << 2009 << 2010);

		QStringList formatted;
		Q_FOREACH (const ContributorInfo& i, authors)
			formatted << i.Fmt ();
		Ui_.Authors_->setHtml (formatted.join ("<hr />"));

		formatted.clear ();
		Q_FOREACH (const ContributorInfo& i, contribs)
			formatted << i.Fmt ();
		Ui_.Contributors_->setHtml (formatted.join ("<hr />"));

		BuildDiagInfo ();
	}

	void AboutDialog::BuildDiagInfo ()
	{
		QString text = QString ("LeechCraft ") + LEECHCRAFT_VERSION + "\n";
		text += QString ("Built with Qt %1, running with Qt %2\n")
				.arg (QT_VERSION_STR)
				.arg (qVersion ());

		text += QString ("Running on: %1\n").arg (Util::SysInfo::GetOSName ());
		text += "--------------------------------\n\n";

		QStringList loadedModules;
		QStringList unPathedModules;
		PluginManager *pm = Core::Instance ().GetPluginManager ();
		Q_FOREACH (QObject *plugin, pm->GetAllPlugins ())
		{
			const QString& path = pm->GetPluginLibraryPath (plugin);

			IInfo *ii = qobject_cast<IInfo*> (plugin);
			if (path.isEmpty ())
				unPathedModules << ("* " + ii->GetName ());
			else
				loadedModules << ("* " + ii->GetName () + " (" + path + ")");

			IHaveDiagInfo *diagInfo = qobject_cast<IHaveDiagInfo*> (plugin);
			if (diagInfo)
			{
				text += "Diag info for " + ii->GetName () + ":\n";
				text += diagInfo->GetDiagInfoString ();
				text += "\n--------------------------------\n\n";
			}
		}

		text += QString ("Normal plugins:") + "\n" + loadedModules.join ("\n") + "\n\n";
		if (!unPathedModules.isEmpty ())
			text += QString ("Adapted plugins:") + "\n" + unPathedModules.join ("\n") + "\n\n";

		Ui_.DiagInfo_->setPlainText (text);
	}
}
