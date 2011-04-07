#include <string>
#include <vector>

#include <QApplication>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>

#include <QTableWidgetItem>

#include "BAMseek.h"

bool BAMseek::loadFile(const std::string & filename){
  QProgressDialog progress("Indexing file.", "Cancel file load.", 0, 100, this);
  progress.setWindowModality(Qt::WindowModal);
  
  slider->setRange(1, 1);
  spinbox->setRange(1,1);
  slider->setValue(1);
  spinbox->setValue(1);

  pr.reset(new PageReader(filename, &progress));
  if(pr->invalid){
    pr.reset(0);
    header->clear();
    header->append(tr("Unable to recognize file as a BAM or SAM file"));
    header->append(tr(filename.c_str()));
    return false;
  }
      
  setWindowTitle(tr(filename.c_str()));
  header->clear();
  header->append(tr(pr->getHeader().c_str()));
    
  slider->setRange(1, pr->getNumPages());
  spinbox->setRange(1,pr->getNumPages());
  slider->setValue(1);
  spinbox->setValue(1);
  
  jumpToPage(1);
  
  return true;
}

bool BAMseek::jumpToPage(int page_no){
  tableview->clear();

  if(!pr.get()) return false;
  if(!pr->jumpToPage(page_no)) return false;
  
  std::vector<std::string> fields;
  int row = 0;
  int col_no = 0;
  
  while(pr->getNextRecord(fields)){
    if((int)fields.size() > col_no){
      col_no = fields.size();
      tableview->setColumnCount(col_no);
    }
    for(size_t i = 0; i < fields.size(); i++){
      QTableWidgetItem * item = new QTableWidgetItem(tr(fields.at(i).c_str()));
      //item->setForeground(Qt::red);
      //item->setForeground(QColor::fromRgbF(0,1,0,1));
      //item->setToolTip(tr("<font color=\"blue\">This is a</font> <b>tool</b> tip"));
      tableview->setItem(row, i, item);
    }
    row++;
  }
  
  setupLabels(col_no);
  return true;
}

BAMseek::BAMseek(QWidget *parent) : QMainWindow(parent), pr(0)
{
  setAttribute(Qt::WA_DeleteOnClose);

  setupFileMenu();
  setupHelpMenu();
  setWindowTitle(tr("BAMseek"));

  setupHeader();
  setupTable();
  setupPager();

  QGroupBox * groupbox = new QGroupBox(tr("Page Number"));
  QHBoxLayout * horiz = new QHBoxLayout;
  horiz->addWidget(spinbox);
  horiz->addWidget(slider);
  groupbox->setLayout(horiz);
  
  QVBoxLayout * vert = new QVBoxLayout;
  vert->addWidget(tableview, 1);
  vert->addWidget(groupbox, 0);

  QWidget * layout = new QWidget;
  layout->setLayout(vert);
  setCentralWidget(layout);
  
}

void BAMseek::setupTable(){
  tableview = new QTableWidget;
  tableview->setRowCount(PAGE_SIZE);
  tableview->setColumnCount(11);
  tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
  //QObject::connect(tableview, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(openFile()));
  
  setupLabels(0);
}

void BAMseek::setupLabels(int col_no){
  QStringList labels;
  labels << "Name";
  labels << "Flag";
  labels << "Reference";
  labels << "Position";
  labels << "Map Quality";
  labels << "Cigar";
  labels << "Next Reference";
  labels << "Next Position";
  labels << "Template Length";
  labels << "Sequence";
  labels << "Quality";

  int col = 11;
  while(col < col_no){
    labels << "Flag";
    col++;
  }

  tableview->setHorizontalHeaderLabels(labels);
}

void BAMseek::setupPager(){
  slider = new QSlider(Qt::Horizontal);
  slider->setRange(1, 100);
  
  spinbox = new QSpinBox;
  spinbox->setRange(1,100);
  
  QObject::connect(slider, SIGNAL(valueChanged(int)), spinbox, SLOT(setValue(int)));
  QObject::connect(spinbox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(jumpToPage(int)));
    
  spinbox->setValue(1);
}

void BAMseek::setupFileMenu(){
  QMenu *fileMenu = new QMenu(tr("&File"), this);
  menuBar()->addMenu(fileMenu);
      
  fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()),
		                        QKeySequence::Open);
  
  fileMenu->addAction(tr("&Dock/Undock Header"), this, SLOT(dockHeader()));
}

void BAMseek::setupHelpMenu(){
  QMenu *helpMenu = new QMenu(tr("&Help"), this);
  menuBar()->addMenu(helpMenu);
  
  helpMenu->addAction(tr("&About BAMseek"), this, SLOT(about()));
  helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
}

void BAMseek::openFile(const QString & path){
  QString fileName = path;

  if (fileName.isNull())
    fileName = QFileDialog::getOpenFileName(this, tr("Open a BAM or a SAM file"), "", "BAM/SAM Files (*.bam *.sam)");

  if (!fileName.isEmpty()) {
    
    if(pr.get()){
      BAMseek * other = new BAMseek;
      other->resize(1200, 800);
      other->show();
      other->loadFile(fileName.toStdString());
      other->activateWindow();
    }else{
      loadFile(fileName.toStdString());
      this->activateWindow();
    }
  }
}

void BAMseek::setupHeader(){
  header = new QTextEdit;
  header->setReadOnly(true);
  header->clear();
  header->append(tr("BAMseek allows you to scroll through large SAM/BAM alignment files.  Please go to File > Open to get started."));

  headerWindow = new QDockWidget(tr("Header"), this);
  headerWindow->setWidget(header);
  headerWindow->setAllowedAreas(Qt::TopDockWidgetArea);
  headerWindow->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::TopDockWidgetArea, headerWindow);
}

void BAMseek::dockHeader(){
  if(headerWindow->isFloating()){
    headerWindow->setFloating(false);
  }else{
    headerWindow->setFloating(true);
  }
}

void BAMseek::about(){
  
  QMessageBox::about(this, tr("About BAMseek"),
                     tr("<img src=\":/Images/logo.png\"><center>BAMseek Version 1.0</center>" \
			"<center>Written by Justin T. Brown</center>" \
			"<center>GPL License</center>" \
			"<center>Copyright 2011, All Rights Reserved</center>" \
			"<p><b>BAMseek</b> allows you to open and explore " \
                        "BAM and SAM files, no matter how big they might be.</p>"));
}
