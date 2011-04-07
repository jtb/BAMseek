#ifndef BAMSEEK_H_
#define BAMSEEK_H_

#include <memory>

#include <QMainWindow>
#include <QDockWidget>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QTextEdit>
#include <QSlider>
#include <QSpinBox>

#include "../FileParse/page_reader.h"

class BAMseek : public QMainWindow
{
  Q_OBJECT

    public:
  BAMseek(QWidget * parent = 0);

  private slots:
  void about();
  void openFile(const QString &path = QString());
  void dockHeader();
  bool jumpToPage(int page_no);
    
  private:
  void setupFileMenu();
  void setupHelpMenu();
  void setupTable();
  void setupLabels(int col_no);
  void setupHeader();
  void setupPager();

  bool loadFile(const std::string & filename);
    
  QDockWidget * headerWindow;
  QTextEdit * header;
  QTableWidget * tableview;
  QStandardItemModel * model;
  QSlider * slider;
  QSpinBox * spinbox;

  std::auto_ptr<PageReader> pr;
};

#endif
