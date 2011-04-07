#include <QApplication>

#include "GUI/BAMseek.h"

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(BAMseek);
  
  QApplication app(argc, argv);
  app.setOrganizationName("Justin Brown");
  app.setApplicationName("BAMseek");
  BAMseek * window = new BAMseek;
  window->resize(1200, 800);
  window->show();
  return app.exec();
}
