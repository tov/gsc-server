/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "ImagesWidget.h"

#include <Wt/WImage.h>

using namespace Wt;

const int ImagesWidget::HURRAY = -1;

ImagesWidget::ImagesWidget(int maxGuesses) {
  for (int i = 0; i <= maxGuesses; ++i) {
    std::string fname = "/images/hangman";
    fname += std::to_string(i) + ".jpg";
    WImage *theImage = addNew<WImage>(fname);
    images_.push_back(theImage);

    // Although not necessary, we can avoid flicker (on konqueror)
    // by presetting the image size.
    theImage->resize(256, 256);
    theImage->hide();
  }

  WImage *hurray = addNew<WImage>("/images/hangmanhurray.jpg");
  hurray->hide();
  images_.push_back(hurray);

  image_ = 0;
  showImage(maxGuesses);
}

void ImagesWidget::showImage(int index) {
  image(image_)->hide();
  image_ = index;
  image(image_)->show();
}

WImage *ImagesWidget::image(int index) const {
  return index == HURRAY ? images_.back() : images_[index];
}
