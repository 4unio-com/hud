#include <QtGMenuImporter.h>
#include <internal/QtGMenuImporterPrivate.h>

#include <QIcon>
#include <QMenu>

using namespace qtgmenu;

QtGMenuImporter::QtGMenuImporter( const QString &service, const QString &path, QObject *parent )
    : QObject( parent ),
      d( new QtGMenuImporterPrivate( service, path, *this ) )
{
}

QtGMenuImporter::~QtGMenuImporter()
{
}

GMenuModel* QtGMenuImporter::GetGMenuModel() const
{
  return d->GetGMenuModel();
}

GActionGroup* QtGMenuImporter::GetGActionGroup() const
{
  return d->GetGActionGroup();
}

std::shared_ptr< QMenu > QtGMenuImporter::GetQMenu() const
{
  return d->GetQMenu();
}

void QtGMenuImporter::ForceRefresh()
{
  d->StartPolling( 100 );
}

#include <QtGMenuImporter.moc>
