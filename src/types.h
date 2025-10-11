/* Copyright (c) 2013, Bastien Dejean
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SXHKD_TYPES_H
#define SXHKD_TYPES_H

#include <xcb/xcb_keysyms.h>
#include <stdbool.h>
#include <stdint.h>
#include "helpers.h"

#define KEYSYMS_PER_KEYCODE  4
#define MOD_STATE_FIELD      0b11111111
#define ESCAPE_KEYSYM        0xff1b
#define SYNCHRONOUS_CHAR     ';'

#if __STDC_VERSION__ >= 202311L
typedef typeof(uint8_t) event_type_t;
typedef typeof(uint16_t) modfield_t;

enum : uint16_t {
	MOD_SHIFT = 1u << 0,
	MOD_LOCK  = 1u << 1,
	MOD_CTRL  = 1u << 2,
	MOD_ALT   = 1u << 3,
	MOD_SUPER = 1u << 6
};
#else
typedef uint8_t event_type_t;
typedef uint16_t modfield_t;

#define MOD_SHIFT (1u << 0)
#define MOD_LOCK  (1u << 1)
#define MOD_CTRL  (1u << 2)
#define MOD_ALT   (1u << 3)
#define MOD_SUPER (1u << 6)
#endif

typedef struct chord_t chord_t;
struct chord_t {
	char repr[MAXLEN];
	xcb_keysym_t keysym;
	xcb_button_t button;
	modfield_t modfield;
	event_type_t event_type;
	bool replay_event;
	bool lock_chain;
	chord_t *next;
	chord_t *more;
};

typedef struct {
	chord_t *head;
	chord_t *tail;
	chord_t *state;
} chain_t;

typedef struct {
	int period;
	int delay;
} cycle_t;

typedef struct hotkey_t hotkey_t;
struct hotkey_t {
	chain_t *chain;
	char command[2 * MAXLEN];
	bool sync;
	cycle_t *cycle;
	hotkey_t *next;
	hotkey_t *prev;
};

typedef struct {
	char *name;
	xcb_keysym_t keysym;
} keysym_dict_t;

NODISCARD __attribute__((hot))
hotkey_t *find_hotkey(xcb_keysym_t keysym, xcb_button_t button, modfield_t modfield, event_type_t event_type, bool *replay_event);
NODISCARD __attribute__((pure))
bool match_chord(chord_t *chord, event_type_t event_type, xcb_keysym_t keysym, xcb_button_t button, modfield_t modfield);
bool chains_interfere(chain_t* a, chain_t* b);
chord_t *make_chord(xcb_keysym_t keysym, xcb_button_t button, modfield_t modfield, event_type_t event_type, bool replay_event, bool lock_chain);
void add_chord(chain_t *chain, chord_t *chord);
chain_t *make_chain(void);
cycle_t *make_cycle(int delay, int period);
hotkey_t *make_hotkey(chain_t *chain, char *command);
void add_hotkey(hotkey_t *hk);
void abort_chain(void);
void destroy_chain(chain_t *chain);
void destroy_chord(chord_t *chord);

#endif
