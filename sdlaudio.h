/*
   Copyright (C) 2009 Todd Kirby (ffmpeg.php@gmail.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SDLAUDIO_H
#define SDLAUDIO_H

typedef struct OpaqueSound *Sound;

int sdlaudio_init_audio();
void sdlaudio_close_audio();
void sdlaudio_play_sound(const Sound sound);
struct OpaqueSound* sdlaudio_load_sound(const char *filename, int channel);
void sdlaudio_free_sound(Sound sound);

#endif
