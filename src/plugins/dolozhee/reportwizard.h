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

#include <QWizard>
#include <interfaces/core/icoreproxy.h>

class QAuthenticator;
class QNetworkReply;
class QNetworkAccessManager;

namespace LeechCraft
{
struct Entity;

namespace Dolozhee
{
	class ChooseUserPage;
	class ReportTypePage;
	class BugReportPage;
	class FeatureRequestPage;

	class ReportWizard : public QWizard
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		QNetworkAccessManager *NAM_;
		ChooseUserPage *ChooseUser_;
		ReportTypePage *ReportType_;
		BugReportPage *BugReportPage_;
		FeatureRequestPage *FRPage_;
		bool FirstAuth_;
	public:
		enum PageID
		{
			ChooseUser,
			UserStatus,
			ReportType,
			BugDetails,
			FeatureDetails,
			Final
		};

		ReportWizard (ICoreProxy_ptr, QWidget* = 0);

		QNetworkAccessManager* GetNAM () const;
		QNetworkReply* PostRequest (const QString&, const QByteArray&);

		ChooseUserPage* GetChooseUserPage () const;
		ReportTypePage* GetReportTypePage () const;
		BugReportPage* GetBugReportPage () const;
		FeatureRequestPage* GetFRPage () const;
	private slots:
		void handleAuthenticationRequired (QNetworkReply*, QAuthenticator*);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
