#ifndef QTGMENUEXPORTER_H
#define QTGMENUEXPORTER_H

#include <QObject>
#include <QDBusConnection>
#include <memory>

class QAction;
class QMenu;

namespace qtgmenu
{

class QtGMenuExporterPrivate;

class QtGMenuExporter final : public QObject
{
    Q_OBJECT

public:
    QtGMenuExporter(const QString& dbusObjectPath, QMenu* menu, const QDBusConnection& dbusConnection = QDBusConnection::sessionBus());
    ~QtGMenuExporter();

    void activateAction(QAction* action);
    void setStatus(const QString& status);
    QString status() const;

protected:
    virtual QString iconNameForAction(QAction* action);

private Q_SLOTS:
    void doUpdateActions();
    void doEmitLayoutUpdated();
    void slotActionDestroyed(QObject*);

private:
    Q_DISABLE_COPY(QtGMenuExporter)
    std::unique_ptr<QtGMenuExporterPrivate> d;

    friend class QtGMenuExporterPrivate;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTER_H */
