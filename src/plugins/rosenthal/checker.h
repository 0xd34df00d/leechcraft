/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <QObject>
#include <QSet>
#include <interfaces/ispellcheckprovider.h>
#include "hunspell/hunspell.hxx"

class QTextCodec;

namespace LC
{
namespace Rosenthal
{
	class KnownDictsManager;

	class Checker : public QObject
				  , public ISpellChecker
	{
		Q_OBJECT
		Q_INTERFACES (ISpellChecker)

		struct HunspellItem
		{
			std::unique_ptr<Hunspell> Hunspell_;
			QTextCodec *Codec_ = nullptr;
		};
		std::vector<HunspellItem> Hunspells_;

		const KnownDictsManager * const KnownMgr_;

		QSet<QString> LearntWords_;
	public:
		Checker (const KnownDictsManager*);

		QStringList GetPropositions (const QString&) const;
		bool IsCorrect (const QString&) const;
		void LearnWord (const QString&);
	public slots:
		void setLanguages (const QStringList&);
	};
}
}
