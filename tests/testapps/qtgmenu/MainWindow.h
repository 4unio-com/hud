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
  std::vector< QMenu* > m_menus;
  QMetaObject::Connection m_refresh_connection;
};

#endif
