#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <libqtgmenu/QtGMenuImporter.h>

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  MainWindow();

private:
  qtgmenu::QtGMenuImporter m_menu_importer;
  std::shared_ptr< QMenu > m_menu;
};

#endif
