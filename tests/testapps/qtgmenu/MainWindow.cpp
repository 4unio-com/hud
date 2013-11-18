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

void MainWindow::RefreshMenus()
{
  menuBar()->clear();

  m_top_menu = m_menu_importer.GetQMenu();
  if( m_top_menu )
  {
    menuBar()->addActions( m_top_menu->actions() );
  }

  menuBar()->repaint();
}
