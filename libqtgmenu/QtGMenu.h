#ifndef QTGMENU_H_
#define QTGMENU_H_

#include <QDBusConnection>
#include <QObject>

class QtGMenuAdaptor;

class _GMenu;
typedef struct _GMenu GMenu;

class QtGMenu : public QObject
{
Q_OBJECT

public:
    explicit QtGMenu(const QDBusConnection &connection, QObject *parent = 0);
    virtual ~QtGMenu();

public Q_SLOTS:
    bool IsPositiveInt(int integer);

Q_SIGNALS:
    void Calculating();

private:
    QDBusConnection m_connection;
    QScopedPointer<QtGMenuAdaptor> m_adaptor;
};

#endif // QTGMENU_H_
