#ifndef SKINENGINE_H
#define SKINENGINE_H
#include <vector>
#include <QMap>
#include <QString>
#include <QList>

class QAction;
class QTabWidget;

namespace LeechCraft
{
	class SkinEngine
	{
		QString OldIconSet_;
		typedef QMap<int, QString> sizef_t;
		QMap<QString, QMap<int, QString> > IconName2Path_;
		QMap<QString, QString> IconName2FileName_;

		SkinEngine ();
	public:
		static SkinEngine& Instance ();
		virtual ~SkinEngine ();

		void UpdateIconSet (const QList<QAction*>&);
		void UpdateIconSet (const QList<QTabWidget*>&);
	private:
		void FindIcons ();
		std::vector<int> GetDirForBase (const QString&, const QString&);
	};
};

#endif

