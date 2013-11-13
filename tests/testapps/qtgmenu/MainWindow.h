#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <libqtgmenu/QtGMenuImporter.h>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

private Q_SLOTS:
  bool RefreshMenus();

private:
  qtgmenu::QtGMenuImporter m_menu_importer;
  std::vector< std::shared_ptr< QMenu > > m_menus;
  QMetaObject::Connection m_refresh_connection;
};

#endif
