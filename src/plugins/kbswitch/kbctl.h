/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QHash>

typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;

namespace LeechCraft
{
namespace KBSwitch
{
	class RulesStorage;

	class KBCtl : public QObject
	{
		Q_OBJECT

		Display *Display_ = 0;
		int XkbEventType_;

		Qt::HANDLE Window_;
		Qt::HANDLE NetActiveWinAtom_;

		bool ExtWM_ = false;

		QStringList Groups_;
		QHash<QString, QString> Variants_;

		QStringList Options_;

		QHash<Qt::HANDLE, uchar> Win2Group_;

		RulesStorage *Rules_;

		bool ApplyScheduled_ = false;

		KBCtl ();
	public:
		enum class SwitchPolicy
		{
			Global,
			PerWindow
		};
	private:
		SwitchPolicy Policy_;
	public:
		static KBCtl& Instance ();
		void Release ();

		void SetSwitchPolicy (SwitchPolicy);

		int GetCurrentGroup () const;

		const QStringList& GetEnabledGroups () const;
		void SetEnabledGroups (QStringList);
		QString GetGroupVariant (const QString&) const;
		void SetGroupVariants (const QHash<QString, QString>&);

		int GetMaxEnabledGroups () const;

		QString GetLayoutName (int group) const;
		QString GetLayoutDesc (int group) const;

		void SetOptions (const QStringList&);

		const RulesStorage* GetRulesStorage () const;

		bool Filter (XEvent*);
	private:
		void HandleXkbEvent (XEvent*);
		void SetWindowLayout (Qt::HANDLE);

		void InitDisplay ();
		void CheckExtWM ();
		void SetupNonExtListeners ();

		void UpdateGroupNames ();

		void AssignWindow (Qt::HANDLE);

		void ApplyKeyRepeat ();
	public slots:
		void scheduleApply ();
		void apply ();
	signals:
		void groupChanged (int);
	};
}
}
