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


#include "k9prefpreview.h"

k9prefPreview::k9prefPreview(QWidget* parent)
: QWidget(parent)
{ 
   Ui_prefPreview.setupUi(this);
   // Only the built-in viewer and mpv remain.
   Ui_prefPreview.rbMplayer->hide();
   Ui_prefPreview.rbXine->hide();
   Ui_prefPreview.rbPhonon->setText(QStringLiteral("mpv"));
   load();
}

k9prefPreview::~k9prefPreview()
{
}

/*$SPECIALIZATION$*/

void k9prefPreview::load() {
    k9Config config;


    Ui_prefPreview.rbPhonon->setChecked(config.getUseMpv());
    Ui_prefPreview.rbInternal->setChecked(!config.getUseMpv());
    
    if (Ui_prefPreview.rbPhonon->isChecked()) 
	rbPhononClick();
    else
	rbInternalClick();

}



void k9prefPreview::save() {
    k9Config config;

    config.setUseMpv( Ui_prefPreview.rbPhonon->isChecked());

    config.save();
}


void k9prefPreview::rbPhononClick() {
  Ui_prefPreview.wsOptions->setCurrentIndex(0); // mpv has no options page
}

void k9prefPreview::rbInternalClick() {
  Ui_prefPreview.wsOptions->setCurrentIndex(0);
}

void k9prefPreview::rbMplayerClick() {
  Ui_prefPreview.wsOptions->setCurrentIndex(1);
}

void k9prefPreview::rbXineClick() {
  Ui_prefPreview.wsOptions->setCurrentIndex(2);
}


