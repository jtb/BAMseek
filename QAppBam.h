#include <QApplication>
#include <QFileOpenEvent>
#include <QEvent>
#include <assert.h>

#include "GUI/BAMseek.h"

class QAppBAM : public QApplication
{
  Q_OBJECT
    
    public:
  QAppBAM(int & argc, char ** argv) : QApplication(argc, argv), window(new BAMseek) {
    window->resize(1200, 800);
    window->show();
    
  }
  
  BAMseek * window;
  
 protected:
  bool event(QEvent * event){
    switch (event->type()) {
    case QEvent::FileOpen:{
      QString file = static_cast<QFileOpenEvent *>(event)->file();
      window->openFile(file);
      return true;
    }
    default:
      return QApplication::event(event);
    }
    return false;
  }
  
};
