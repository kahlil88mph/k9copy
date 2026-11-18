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
   load();
}

k9prefPreview::~k9prefPreview()
{
}

/*$SPECIALIZATION$*/

void k9prefPreview::load() {
    k9Config config;


    Ui_prefPreview.rbMpv->setChecked(config.getUseMpv());
    Ui_prefPreview.rbInternal->setChecked(!config.getUseMpv());
    
    if (Ui_prefPreview.rbMpv->isChecked()) 
	rbMpvClick();
    else
	rbInternalClick();

}



void k9prefPreview::save() {
    k9Config config;

    config.setUseMpv( Ui_prefPreview.rbMpv->isChecked());

    config.save();
}


void k9prefPreview::rbMpvClick() {
  Ui_prefPreview.wsOptions->setCurrentIndex(0); // mpv has no options page
}

void k9prefPreview::rbInternalClick() {
  Ui_prefPreview.wsOptions->setCurrentIndex(0);
}




