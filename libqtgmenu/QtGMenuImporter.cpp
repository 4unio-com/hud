#include <QtGMenuImporter.h>
#include <internal/QtGMenuImporterPrivate.h>

#include <QIcon>
#include <QMenu>

namespace qtgmenu
{

QtGMenuImporter::QtGMenuImporter( const QString &service, const QString &path, QObject *parent )
    : QObject( parent ),
      d( new QtGMenuImporterPrivate( service, path, *this ) )
{
}

QtGMenuImporter::~QtGMenuImporter()
{
}

GMenu* QtGMenuImporter::GetGMenu() const
{
  return d->GetGMenu();
}

std::shared_ptr< QMenu > QtGMenuImporter::GetQMenu() const
{
  return d->GetQMenu();
}


void QtGMenuImporter::ForceRefresh()
{
  d->StartPolling( 100 );
}

int QtGMenuImporter::GetItemCount()
{
  GMenu* menu = d->GetGMenu();

  if( !menu )
  {
    return 0;
  }

  gint item_count = g_menu_model_get_n_items( G_MENU_MODEL( menu ) );
  g_object_unref( menu );

  return item_count;
}

} // namespace qtgmenu

#include <QtGMenuImporter.moc>
