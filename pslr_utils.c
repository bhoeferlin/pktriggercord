/*
    pkTriggerCord
    Remote control of Pentax DSLR cameras.
    Copyright (C) 2011-2020 Andras Salamon <andras.salamon@melda.info>

    based on:

    pslr-shoot

    Command line remote control of Pentax DSLR cameras.
    Copyright (C) 2009 Ramiro Barreiro <ramiro_barreiro69@yahoo.es>
    With fragments of code from PK-Remote by Pontus Lidman.
    <https://sourceforge.net/projects/pkremote>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    and GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _WIN32
#include "usleep.h"
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include <math.h>
#include <stdio.h>

#include "pslr.h"
#include "pslr_utils.h"

double timeval_diff_sec(struct timeval* t2, struct timeval* t1)
{
    if (!t2 || !t1) {
        return 0.0;
    }

    return ((double)(t2->tv_usec - t1->tv_usec) / 1000000.0) + (double)(t2->tv_sec - t1->tv_sec);
}

void sleep_sec(double sec)
{
    int i;
    double whole;
    double frac;

    if (sec <= 0.0) {
        return;
    }

    whole = floor(sec);
    frac = sec - whole;

    for (i = 0; i < (int)whole; ++i) {
        usleep(999999); /* keep below 1 second for Windows compatibility */
    }

    if (frac > 0.0) {
#ifdef _WIN32
        usleep((__int64)(1000000.0 * frac));
#else
        usleep((useconds_t)(1000000.0 * frac));
#endif
    }
}

pslr_rational_t parse_aperture(char* aperture_str)
{
    char C = '\0';
    float F = 0.0f;
    pslr_rational_t aperture = { 0, 0 };

    if (!aperture_str) {
        return aperture;
    }

#if defined(_MSC_VER)
    if (sscanf_s(aperture_str, "%f%c", &F, &C, 1) != 1) {
        F = 0.0f;
    }
#else
    if (sscanf(aperture_str, "%f%c", &F, &C) != 1) {
        F = 0.0f;
    }
#endif

    /* It's unlikely that you want an f-number > 100, even for a pinhole.
       On the other hand, the fastest lens I know of is a f:0.8 Zeiss */
    if (F > 100.0f || F < 0.8f) {
        F = 0.0f;
    }

    aperture.nom = (int)(F * 10.0f);
    aperture.denom = 10;

    return aperture;
}

pslr_rational_t parse_shutter_speed(char* shutter_speed_str)
{
    char C = '\0';
    float F = 0.0f;
    pslr_rational_t shutter_speed = { 0, 0 };

    if (!shutter_speed_str) {
        return shutter_speed;
    }

#if defined(_MSC_VER)
    if (sscanf_s(shutter_speed_str, "%d/%d%c",
            &shutter_speed.nom, &shutter_speed.denom, &C, 1)
        == 2) {
        /* noop */
    } else if (sscanf_s(shutter_speed_str, "%d%c",
                   &shutter_speed.nom, &C, 1)
        == 1) {
        shutter_speed.denom = 1;
    } else if (sscanf_s(shutter_speed_str, "%f%c",
                   &F, &C, 1)
        == 1) {
        F *= 1000.0f;
        shutter_speed.denom = 1000;
        shutter_speed.nom = (int)F;
    } else {
        shutter_speed.nom = 0;
        shutter_speed.denom = 0;
    }
#else
    if (sscanf(shutter_speed_str, "%d/%d%c",
            &shutter_speed.nom, &shutter_speed.denom, &C)
        == 2) {
        /* noop */
    } else if (sscanf(shutter_speed_str, "%d%c",
                   &shutter_speed.nom, &C)
        == 1) {
        shutter_speed.denom = 1;
    } else if (sscanf(shutter_speed_str, "%f%c",
                   &F, &C)
        == 1) {
        F *= 1000.0f;
        shutter_speed.denom = 1000;
        shutter_speed.nom = (int)F;
    } else {
        shutter_speed.nom = 0;
        shutter_speed.denom = 0;
    }
#endif

    return shutter_speed;
}