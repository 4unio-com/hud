#include "QtGMenuExporter.h"

#include <QMenu>

namespace qtgmenu
{

class QtGMenuExporterPrivate
{
};

QtGMenuExporter::QtGMenuExporter(const QString& objectPath, QMenu* menu, const QDBusConnection& _connection)
    : QObject(menu),
      d(new QtGMenuExporterPrivate)
{
}

QtGMenuExporter::~QtGMenuExporter()
{
    delete d;
}

void QtGMenuExporter::doUpdateActions()
{
}

void QtGMenuExporter::doEmitLayoutUpdated()
{
}

QString QtGMenuExporter::iconNameForAction(QAction* action)
{
    return QString();
}

void QtGMenuExporter::activateAction(QAction* action)
{
}

void QtGMenuExporter::slotActionDestroyed(QObject* object)
{
}

void QtGMenuExporter::setStatus(const QString& status)
{
}

QString QtGMenuExporter::status() const
{
    return QString();
}

} // namespace qtgmenu

#include "QtGMenuExporter.moc"
