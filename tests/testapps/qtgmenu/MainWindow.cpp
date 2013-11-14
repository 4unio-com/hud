#include <MainWindow.h>

#include <QtWidgets>

MainWindow::MainWindow()
    : m_menu_importer( "org.gnome.Gedit", "/com/canonical/unity/gtk/window/0" )
{
  m_refresh_connection = connect( &m_menu_importer, SIGNAL( MenuItemsChanged() ), this,
      SLOT( RefreshMenus() ) );
}

MainWindow::~MainWindow()
{
  disconnect( m_refresh_connection );
}

bool MainWindow::RefreshMenus()
{
  menuBar()->clear();

  m_menus = m_menu_importer.GetQMenus();
  for( auto& menu : m_menus )
  {
    menuBar()->addMenu( menu );
  }
}
