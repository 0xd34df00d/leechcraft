#ifndef PLUGINS_LMP_ENTITYCHECKER_H
#define PLUGINS_LMP_ENTITYCHECKER_H
#include <memory>
#include <QObject>
#include "phonon.h"

namespace LeechCraft
{
	struct DownloadEntity;

	namespace Plugins
	{
		namespace LMP
		{
			class EntityChecker : public QObject
			{
				Q_OBJECT

				bool Result_;
				bool Break_;
			public:
				EntityChecker (const LeechCraft::DownloadEntity&);
				bool Can () const;
			private slots:
				void stateChanged (Phonon::State);
			};
		};
	};
};

#endif

