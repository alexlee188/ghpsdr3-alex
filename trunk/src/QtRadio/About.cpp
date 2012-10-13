
/* Copyright (C) 2012 - Alex Lee, 9V1Al
* modifications of the original program by John Melton
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Pl
*/

#include "About.h"
#include "ui_About.h"

#include <QString>

#if !defined GITREV
#define GITREV "."
#endif

static const char git [] = GITREV;

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    ui->textBrowserGit->setHtml(QString(git).replace('!', "<br>") );   
}

About::~About()
{
    delete ui;
}
