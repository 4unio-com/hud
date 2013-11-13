#include <MainWindow.h>

#include <QtWidgets>

MainWindow::MainWindow()
    : m_menu_importer( "org.gnome.Terminal.Display_0", "/com/canonical/unity/gtk/window/0" )
{
  QEventLoop menu_appeared_wait;
  menu_appeared_wait.connect( &m_menu_importer, SIGNAL( MenuAppeared() ), SLOT( quit() ) );
  menu_appeared_wait.exec();

  m_refresh_connection = connect( &m_menu_importer, SIGNAL( MenuItemsChanged() ), this,
      SLOT( RefreshMenus() ) );
}

MainWindow::~MainWindow()
{
  disconnect (m_refresh_connection);
}

bool MainWindow::RefreshMenus()
{
  menuBar()->clear();

  m_menus = m_menu_importer.GetQMenus();
  for( auto& menu : m_menus )
  {
    menuBar()->addMenu( menu.get() );
  }
}
