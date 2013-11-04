#include "QtGMenuImporter.h"

#include <QIcon>

namespace qtgmenu
{

class QtGMenuImporterPrivate
{
};

QtGMenuImporter::QtGMenuImporter(const QString& service, const QString& path, QObject* parent)
    : QObject(parent),
      d(new QtGMenuImporterPrivate)
{
}

QtGMenuImporter::~QtGMenuImporter()
{
    delete d;
}

void QtGMenuImporter::slotLayoutUpdated(uint revision, int parentId)
{
}

void QtGMenuImporter::processPendingLayoutUpdates()
{
}

QMenu* QtGMenuImporter::menu() const
{
    return nullptr;
}

void QtGMenuImporter::slotItemActivationRequested(int id, uint timestamp)
{
}

void QtGMenuImporter::slotItemsPropertiesUpdated()
{
}

void QtGMenuImporter::slotGetLayoutFinished(QDBusPendingCallWatcher* watcher)
{
}

void QtGMenuImporter::sendClickedEvent(int id)
{
}

void QtGMenuImporter::updateMenu()
{
}

void QtGMenuImporter::slotMenuAboutToShow()
{
}

void QtGMenuImporter::slotAboutToShowDBusCallFinished(QDBusPendingCallWatcher* watcher)
{
}

void QtGMenuImporter::slotMenuAboutToHide()
{
}

QMenu* QtGMenuImporter::createMenu(QWidget* parent)
{
    return nullptr;
}

QIcon QtGMenuImporter::iconForName(const QString& name)
{
    return QIcon();
}

} // namespace qtgmenu

#include "QtGMenuImporter.moc"
