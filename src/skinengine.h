#ifndef SKINENGINE_H
#define SKINENGINE_H
#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QAction>

namespace LeechCraft
{
	class SkinEngine : public QObject
	{
		Q_OBJECT

		QString OldIconSet_;
		QMap<QString, QString> IconName2Path_;
		QMap<QString, QString> IconName2FileName_;

		SkinEngine ();
	public:
		static SkinEngine& Instance ();
		virtual ~SkinEngine ();
	private:
		void FindIcons ();
		std::vector<int> GetDirForBase (const QString&, const QString&);
	public slots:
		void updateIconSet (const QList<QAction*>&);
	};
};

#endif

