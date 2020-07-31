/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef STARTUPWIZARD_H
#define STARTUPWIZARD_H
#include <QWizard>
#include <QMap>

namespace LC
{
	class StartupWizard : public QWizard
	{
		Q_OBJECT

		QList<QWizardPage*> Pages_;
		QMap<QWizardPage*, int> Page2ID_;
		int TypeChoseID_;
	public:
		enum Type
		{
			TBasic,
			TAdvanced
		};
	private:
		Type Type_;
	public:
		StartupWizard (QWidget* = 0);

		int nextId () const;
	private:
		void AddPages ();
	private slots:
		void handleTypeChanged (StartupWizard::Type);

		void handleAccepted ();
		void handleRejected ();
	};
};

#endif

