#include <MainWindow.h>

#include <libqtgmenu/QtGMenuImporter.h>

#include <QtWidgets>

using namespace qtgmenu;

MainWindow::MainWindow()
{
  QtGMenuImporter importer( "org.gnome.Terminal.Display_0", "/com/canonical/unity/gtk/window/0" );

  QEventLoop menu_appeared_wait;
  menu_appeared_wait.connect( &importer, SIGNAL( MenuAppeared() ), SLOT( quit() ) );
  menu_appeared_wait.exec();

  auto menu = importer.GetQMenu();
  menuBar()->addMenu( menu.get() );
}
