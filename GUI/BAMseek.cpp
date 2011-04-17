#include <string>
#include <sstream>
#include <vector>

#include <QApplication>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QPixmap>

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
      if(i==9 && fields.size() > 9 && fields.at(10).size() == fields.at(9).size()){
	item->setToolTip(prettyPrintBaseQual(fields.at(9), fields.at(10)).c_str());
      //item->setToolTip(tr("<font color=\"blue\">This is a</font> <b>tool</b> tip"));
      }
      if(i==1){
	item->setToolTip(prettyPrintFlag(atoi(fields.at(i).c_str())).c_str());
      }
      if(i==5){
	item->setToolTip(prettyPrintCigar(fields.at(5)).c_str());
      }
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
  labels << "Mate Reference";
  labels << "Mate Position";
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
  
  QAction *openAction = fileMenu->addAction(tr("&Open..."));
  openAction->setShortcuts(QKeySequence::Open);
  connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    
  QAction *dockAction = fileMenu->addAction(tr("&Dock/Undock Header"));
  dockAction->setShortcuts(QKeySequence::AddTab);
  connect(dockAction, SIGNAL(triggered()), this, SLOT(dockHeader()));
}

void BAMseek::setupHelpMenu(){
  QMenu *helpMenu = new QMenu(tr("&Help"), this);
  menuBar()->addMenu(helpMenu);
  
  helpMenu->addAction(tr("&About BAMseek"), this, SLOT(about()));
  helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));

  helpMenu->addAction(tr("&Check for updates..."), this, SLOT(openWebLink()));
}

void BAMseek::openWebLink(){
  QUrl url("http://code.google.com/p/bamseek/");
  QDesktopServices::openUrl(url);
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
  
  QWidget * wid = new QWidget;
  QHBoxLayout * horiz = new QHBoxLayout;
  QLabel * label = new QLabel;
  QPixmap pixmap(":/Images/BAMseek.png");
  label->setPixmap(pixmap);
  horiz->addWidget(label, 0);
  horiz->addWidget(header, 1);
  wid->setLayout(horiz);
  headerWindow->setWidget(wid);
  
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
                     tr("<img src=\":/Images/logo.png\"><center>BAMseek Version 0.1.2</center>" \
			"<center>Written by Justin T. Brown</center>" \
			"<center>GPL License</center>" \
			"<center>Copyright 2011, All Rights Reserved</center>" \
			"<p><b>BAMseek</b> allows you to open and explore " \
                        "BAM and SAM files, no matter how big they might be.</p>"));
}

namespace {
  int getNum(const std::string & test, int & pos){
    int ans = 0;
    while(pos < (int)test.size()){
      if(isdigit(test[pos])){
	char num[2] = {test[pos], 0};
	ans = 10*ans + atoi(num);
      }else{
	return ans;
      }
      pos++;
    }
    return 0;
  }

  char getVal(const std::string & test, int & pos){
    return test[pos++];
  }
  
}

std::string prettyPrintCigar(const std::string & cigar){
  std::stringstream ans;
  
  if(cigar == "*"){
    ans << "No alignment information\n";
  }else{
    
    char val;
    int pos = 0;
    while(pos < (int)cigar.size()){
      ans << getNum(cigar, pos) << " ";
      
      switch(val = getVal(cigar, pos)){
      case 'M' : case 'm' : ans << "Match/Mismatch\n"; break;
      case 'I' : case 'i' : ans << "Insertion to reference\n"; break;
      case 'D' : case 'd' : ans << "Deletion from reference\n"; break;
      case 'N' : case 'n' : ans << "Skipped region from reference\n"; break;
      case 'S' : case 's' : ans << "Soft clipping (clipped sequence present)\n"; break;
      case 'H' : case 'h' : ans << "Hard clipping (clipped sequence removed)\n"; break;
      case 'P' : case 'p' : ans << "Padding (silent deletion from padded reference)\n"; break;
      case '=' : ans << "Match\n"; break;
      case 'X' : case 'x' : ans << "Mismatch\n"; break;
      default : ans << val << "\n"; break;
      }

    }

  }
  
  return ans.str();
}

std::string prettyPrintBaseQual(const std::string & bases, const std::string & quals){
  if(bases == "*" || bases.size() != quals.size()) return bases;

  
  std::string hexcolor = "";
  for(size_t i = 0; i < bases.size(); i++){
    hexcolor += "<font size=\"5\" color=\"";

    int c = quals.at(i) - 33;
    if(c < 20) hexcolor += "#E9CFEC"; 
    else hexcolor += "#571B7e";
    
    hexcolor += "\">";
    hexcolor += bases.at(i);
    hexcolor += "</font>";
    
  }

  return hexcolor;
  
}

std::string prettyPrintFlag(int flag){
  if(flag<0) return "";
  bool unmapped = false;
  bool unmappedmate = false;
  bool paired = false;
  std::string answer = "";
  if(flag%2){
    answer+="Read is paired\n";
    paired = true;
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read mapped in proper pair.\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read is unmapped.\n";
    unmapped = true;
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Mate is unmapped.\n";
    unmappedmate = true;
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read is on reverse strand\n";
  }else if(!unmapped){
    answer+="Read is on forward strand\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Mate is on reverse strand\n";
  }else if(paired && !unmappedmate){
    answer+="Mate is on forward strand\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read is first in template.\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read is last in template.\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read is not primary alignment\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read fails platform/vendor quality checks.\n";
  }
  flag = (flag >> 1);
  if(flag%2){
    answer+="Read is PCR or optical duplicate\n";
  }

    
  return answer;
}


