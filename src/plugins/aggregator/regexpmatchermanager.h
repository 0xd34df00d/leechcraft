#ifndef REGEXPMATCHERMANAGER_H
#define REGEXPMATCHERMANAGER_H
#include <deque>
#include <QAbstractItemModel>
#include <QStringList>
#include <stdexcept>
#include "item.h"

struct Item;

class RegexpMatcherManager : public QAbstractItemModel
{
	Q_OBJECT
public:
	typedef std::pair<QString, QString> titlebody_t;
	class AlreadyExists : public std::runtime_error
	{
	public:
		explicit AlreadyExists (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	class NotFound : public std::runtime_error
	{
	public:
		explicit NotFound (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	class Malformed : public std::runtime_error
	{
	public:
		explicit Malformed (const std::string& str)
		: std::runtime_error (str)
		{
		}
	};

	struct RegexpItem
	{
		QString Title_;
		QString Body_;

		RegexpItem (const QString& = QString (),
				const QString& = QString ());
		bool operator== (const RegexpItem&) const;
		bool IsEqual (const QString&) const;
		QByteArray Serialize () const;
		void Deserialize (QByteArray&);
	};
private:
	QStringList ItemHeaders_;
	typedef std::deque<RegexpItem> items_t;
	items_t Items_;

	RegexpMatcherManager ();

	mutable bool SaveScheduled_;
public:
	static RegexpMatcherManager& Instance ();
	virtual ~RegexpMatcherManager ();

	void Release ();
	void Add (const QString&, const QString&);
	void Remove (const QString&);
	void Remove (const QModelIndex&);
	void Modify (const QString&, const QString&);
	titlebody_t GetTitleBody (const QModelIndex&) const;
	void HandleItem (const Item_ptr&) const;

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
private slots:
	void saveSettings () const;
private:
	void RestoreSettings ();
	void ScheduleSave ();
signals:
	void gotLink (const QByteArray&) const;
};

#endif

