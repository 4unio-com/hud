#include <MainWindow.h>

#include <QtWidgets>

MainWindow::MainWindow()
    : m_menu_importer( "org.gnome.Terminal.Display_0", "/com/canonical/unity/gtk/window/0" )
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
  m_top_menu = m_menu_importer.GetQMenu();

  // remove missing items
  for( int i = 0; i < menuBar()->actions().size(); ++i )
  {
    QAction* action = menuBar()->actions().at( i );

    if( !m_top_menu || i >= m_top_menu->actions().size()
        || !m_top_menu->actions().contains( action ) )
    {
      menuBar()->removeAction( action );
      --i;
    }
  }

  if( !m_top_menu )
  {
    return;
  }

  // add new items
  for( int i = 0; i < m_top_menu->actions().size(); ++i )
  {
    QAction* action = m_top_menu->actions().at( i );

    if( i >= menuBar()->actions().size() )
    {
      menuBar()->addAction( action );
    }
    else if( action != menuBar()->actions().at( i ) )
    {
      menuBar()->insertAction( menuBar()->actions().at( i ), action );
    }
  }

  menuBar()->repaint();
}
