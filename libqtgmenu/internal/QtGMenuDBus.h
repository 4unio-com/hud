#ifndef QTGMENUDBUS_H_
#define QTGMENUDBUS_H_

#include <QDBusConnection>
#include <QObject>

class QtGMenuDBusAdaptor;

class _GMenu;
typedef struct _GMenu GMenu;

class QtGMenuDBus : public QObject
{
    Q_OBJECT

public:
    explicit QtGMenuDBus(const QDBusConnection& connection, QObject* parent = 0);
    virtual ~QtGMenuDBus();

public Q_SLOTS:
    bool IsPositiveInt(int integer);

Q_SIGNALS:
    void Calculating();

private:
    QDBusConnection m_connection;
    QScopedPointer<QtGMenuDBusAdaptor> m_adaptor;
};

#endif // QTGMENUDBUS_H_
