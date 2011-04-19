#include "QAppBAM.h"

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(BAMseek);
  
  QAppBAM app(argc, argv);
  app.setOrganizationName("Justin Brown");
  app.setApplicationName("BAMseek");

  qApp->setStyleSheet("QMainWindow::separator { background: #C67171; height: 7px; }");

  return app.exec();
}
