/***************************************************************************
 *   Copyright (C) 2008 by Krzysztof Gantzke       k.gantzke@meilhaus.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "medriver.h"
#include "string.h"

int main(int argc, char *argv[])
{
	char msg[256];
	int i;

	meOpen(0);
		for (i=-128; i<=128; i++)
		{
			meErrorGetMessage(i, msg, 256);
			printf("%d:\t%s\n", i, msg);
		}
	meClose(0);

	return EXIT_SUCCESS;
}
