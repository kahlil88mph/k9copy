//
// C++ Implementation:
//
// Description:
//
// Developer: Pauline123 <pauline123@users.sf.net>, (C) 2014
//
// Author: Jean-Michel PETIT <k9copy@free.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "k9burnprogress.h"
#include <QProgressBar>
#include <QLabel>
#include <QApplication>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QMovie>
#include <QLayout>

#include <QListWidget>
#include "k9guiitem.h"
#include "k9guiitem.h"

k9BurnProgress::k9BurnProgress(QWidget* parent)
        : QDialog(parent) {
    Ui_Progress.setupUi(this);
    setModal(true);
    m_process=new k9Process(this,0);
    m_logView=new k9LogView(this);
    Ui_Progress.image->setPalette(this->palette());
    QGridLayout *l=new QGridLayout(Ui_Progress.image);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_logView,0,0);

    KGuiItem::assign(Ui_Progress.bCancel, KStandardGuiItem::stop() );

    // Qt5 code
    QString aright=QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kmyapp/aright.png");
    QString adown=QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kmyapp/adown.png");
    Ui_Progress.gbOutput->setStyleSheet("*::indicator:unchecked {image:url("+aright+"); } *::indicator:checked {image:url("+adown+"); } ");


}


k9BurnProgress::~k9BurnProgress() {
    delete m_process;
}


void k9BurnProgress::setElapsed(const QString _text) {
    Ui_Progress.lblElapsed->setText(_text);
}

void k9BurnProgress::setTitle(const QString _text) {
    Ui_Progress.lblTitle->setText(_text);
}


void k9BurnProgress::setLabelText(const QString _text) {
    Ui_Progress.LabelText->setText(_text);
}

void k9BurnProgress::setProgress(long _position,long _total) {
    Ui_Progress.ProgressBar->setRange(0,_total);
    Ui_Progress.ProgressBar->setValue(_position);
}

int k9BurnProgress::execute() {
    if (! m_process->isRunning()) {
	qDebug() << m_process->debug();
        if (!m_process->start())
            return -1;
    }

//   m_timer.start(200,FALSE);
    show();
    m_canceled=false;
    //the sync method allows to wait for the process end while receiving stdout.
    m_process->sync();

//   m_timer.stop();
    close();
    if (m_canceled)
	return 0;
    else if (m_process->normalExit())
        return 1;
    else
        return -1;
}


void k9BurnProgress::bCancelClick() {
    m_process->kill();
    m_canceled=true;
}

k9Process* k9BurnProgress::getProcess() const {
    return m_process;
}

/*$SPECIALIZATION$*/



void k9BurnProgress::setMovie(QString _fileName) {
    Q_UNUSED(_fileName)
}


bool k9BurnProgress::getCanceled() const {
    return m_canceled;
}


void k9BurnProgress::gbOutputToggled(bool state) {
   if (!state) {
        m_outputSize=Ui_Progress.image->height();
        this->resize(this->width(),this->height()-m_outputSize);
        m_outputSize-=Ui_Progress.image->height();
        this->setMaximumHeight(this->height());
    } else {
        this->setMaximumHeight(32768);
        this->resize(this->width(),this->height()+m_outputSize);
    }

    Ui_Progress.image->setVisible(state);
}

void k9BurnProgress::closeEvent ( QCloseEvent *  ) {

}
