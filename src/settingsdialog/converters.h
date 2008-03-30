#ifndef CONVERTERS_H
#define CONVERTERS_H
#include <QMap>
#include <QString>
#include <QObject>

class QVariant;
class QWidget;
class SettingsItemInfo;

class Converter : public QObject
{
    Q_OBJECT

    unsigned int Type_;
protected:
    void SetType (unsigned int type);
    QMap<QWidget*, QString> Widget2Property_;
    QMap<QWidget*, QObject*> Widget2Object_;
    QList<QWidget*> ModifiedProperties_;
public:
    Converter (QObject *parent);
    virtual ~Converter ();
    virtual QWidget* Convert (const QVariant&, const SettingsItemInfo&, const QString&, QObject*) = 0;
    virtual bool MakeLabel () const;
    virtual unsigned int GetType () const;
    virtual void UpdateSettings ();
private slots:
    virtual void updateSetting ();
private:
    virtual QVariant ReadSetting (QWidget*) const = 0;
};

class QStringConverter : public Converter
{
    Q_OBJECT
public:
    QStringConverter (QObject *parent);
    virtual ~QStringConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual QVariant ReadSetting (QWidget*) const;
};

class IntConverter : public Converter
{
    Q_OBJECT
public:
    IntConverter (QObject *parent);
    virtual ~IntConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual QVariant ReadSetting (QWidget*) const;
};

class UIntConverter : public Converter
{
    Q_OBJECT
public:
    UIntConverter (QObject *parent);
    virtual ~UIntConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual QVariant ReadSetting (QWidget*) const;
};

class BoolConverter : public Converter
{
    Q_OBJECT
public:
    BoolConverter (QObject *parent);
    virtual ~BoolConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual bool MakeLabel () const;
    virtual QVariant ReadSetting (QWidget*) const;
};

class QStringListConverter : public Converter
{
    Q_OBJECT
public:
    QStringListConverter (QObject *parent);
    virtual ~QStringListConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual bool MakeLabel () const;
    virtual QVariant ReadSetting (QWidget*) const;
};

class PairedStringListConverter : public Converter
{
    Q_OBJECT
public:
    PairedStringListConverter (QObject *parent);
    virtual ~PairedStringListConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual bool MakeLabel () const;
    virtual QVariant ReadSetting (QWidget*) const;
};

class IntRangeConverter : public Converter
{
    Q_OBJECT
public:
    IntRangeConverter (QObject *parent);
    virtual ~IntRangeConverter ();
    virtual QWidget* Convert (const QVariant& value, const SettingsItemInfo&, const QString&, QObject*);
    virtual bool MakeLabel () const;
    virtual QVariant ReadSetting (QWidget*) const;
};

#endif

