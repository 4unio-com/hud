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
  QtGMenuExporter( const QString& dbusObjectPath, QMenu* menu,
      const QDBusConnection& dbusConnection = QDBusConnection::sessionBus() );
  ~QtGMenuExporter();

private Q_SLOTS:

private:
  Q_DISABLE_COPY(QtGMenuExporter)
  std::unique_ptr< QtGMenuExporterPrivate > d;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTER_H */
