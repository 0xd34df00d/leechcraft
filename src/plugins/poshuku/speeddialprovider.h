#ifndef SPEEDDIALPROVIDER_H
#define SPEEDDIALPROVIDER_H
#include <QObject>
#include <QStringList>
#include <QDateTime>

struct HistoryItem;

class SpeedDialProvider : public QObject
{
	Q_OBJECT

	mutable QStringList RenderQueue_;

	SpeedDialProvider ();
public:
	struct Item
	{
		QString URL_;
		int ResX_;
		int ResY_;
		QByteArray Thumb_;
		QDateTime ShotDate_;
	};

	static SpeedDialProvider& Instance ();
	QString GetHTML () const;
private:
	QString GetHTMLForItem (const HistoryItem&) const;
	void Regenerate (const QString&) const;
private slots:
	void handleFinished ();
signals:
	void newThumbAvailable ();
};

#endif

