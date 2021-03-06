/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 by Linus Nielsen Feltzing
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#ifndef PCM_PLAYBACK_H
#define PCM_PLAYBACK_H

#include <string.h> /* size_t */

#define DMA_REC_ERROR_DMA       (-1)
#ifdef HAVE_SPDIF_REC
#define DMA_REC_ERROR_SPDIF     (-2)
#endif

/** Warnings **/
/* pcm (dma) buffer has overflowed */
#define PCMREC_W_PCM_BUFFER_OVF         0x00000001
/* encoder output buffer has overflowed */
#define PCMREC_W_ENC_BUFFER_OVF         0x00000002
/** Errors **/
/* failed to load encoder */
#define PCMREC_E_LOAD_ENCODER           0x80001000
/* error originating in encoder */
#define PCMREC_E_ENCODER                0x80002000
/* filename queue has desynced from stream markers */
#define PCMREC_E_FNQ_DESYNC             0x80004000
/* I/O error has occurred */
#define PCMREC_E_IO                     0x80008000
#ifdef DEBUG
/* encoder has written past end of allocated space */
#define PCMREC_E_CHUNK_OVF              0x80010000
#endif /* DEBUG */

/** RAW PCM routines used with playback and recording **/

/* Typedef for registered callbacks */
typedef void (*pcm_play_callback_type)(unsigned char **start,
                                       size_t *size);
typedef void (*pcm_rec_callback_type)(int status, void **start, size_t *size);

/* set the pcm frequency - use values in hw_sampr_list 
 * when CONFIG_SAMPR_TYPES is #defined, or-in SAMPR_TYPE_* fields with
 * frequency value. SAMPR_TYPE_PLAY is 0 and the default if none is
 * specified. */
#ifdef CONFIG_SAMPR_TYPES
#ifdef SAMPR_TYPE_REC
unsigned int pcm_sampr_type_rec_to_play(unsigned int samplerate);
#endif
#endif /* CONFIG_SAMPR_TYPES */

void pcm_set_frequency(unsigned int samplerate);
/* apply settings to hardware immediately */
void pcm_apply_settings(void);

/** RAW PCM playback routines **/

/* Reenterable locks for locking and unlocking the playback interrupt */
void pcm_play_lock(void);
void pcm_play_unlock(void);

void pcm_init(void) INIT_ATTR;
void pcm_postinit(void);
bool pcm_is_initialized(void);

/* This is for playing "raw" PCM data */
void pcm_play_data(pcm_play_callback_type get_more,
                   unsigned char* start, size_t size);

void pcm_calculate_peaks(int *left, int *right);
const void* pcm_get_peak_buffer(int* count);
size_t pcm_get_bytes_waiting(void);

void pcm_play_stop(void);
void pcm_play_pause(bool play);
bool pcm_is_paused(void);
bool pcm_is_playing(void);

void pcm_play_set_dma_started_callback(void (* callback)(void));

#ifdef HAVE_RECORDING

/** RAW PCM recording routines **/

/* Reenterable locks for locking and unlocking the recording interrupt */
void pcm_rec_lock(void);
void pcm_rec_unlock(void);

/* Initialize pcm recording interface */
void pcm_init_recording(void);
/* Uninitialize pcm recording interface */
void pcm_close_recording(void);

/* Start recording "raw" PCM data */
void pcm_record_data(pcm_rec_callback_type more_ready,
                     void *start, size_t size);

/* Stop tranferring data into supplied buffer */
void pcm_stop_recording(void);

/* Is pcm currently recording? */
bool pcm_is_recording(void);

/* Called by bottom layer ISR when transfer is complete. Returns non-zero
 * size if successful. Setting start to NULL forces stop. */
void pcm_rec_more_ready_callback(int status, void **start, size_t *size);

void pcm_calculate_rec_peaks(int *left, int *right);

#endif /* HAVE_RECORDING */

#endif /* PCM_PLAYBACK_H */
