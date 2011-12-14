/* 
 * File:   G711A.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 29 November 2011, 14:17
 */

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef G711A_H
#define	G711A_H

#include <QObject>
#include <QDebug>

class G711A : public QObject {
    Q_OBJECT
public:
    G711A();
    virtual ~G711A();
    unsigned char encode(short sample);
    short decode(unsigned char sample);

private:
    short decodetable[256];
    unsigned char encodetable[65536];
};

#endif	/* G711A_H */

