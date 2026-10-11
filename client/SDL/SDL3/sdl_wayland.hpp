/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * SDL3 Client - native Wayland interactive window move
 *
 * Copyright 2026 Ibrahim Sevinc <ibrahim.sevinc.mail@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <SDL3/SDL.h>

/* Set a toplevel's layout bounds to its own logical size, excluding subsurfaces.
 * Double-buffered: caller commits the surface (or presents its renderer). */
void sdl_wayland_set_window_geometry(SDL_Window* window);

/* Experimental RAIL popup surfaces; SDL still owns rendering and input. */
bool sdl_wayland_subsurface_create(SDL_Window* window, SDL_Window* parent, int x, int y);
void sdl_wayland_subsurface_destroy(SDL_Window* window);
bool sdl_wayland_subsurface_position(SDL_Window* window, int x, int y);
bool sdl_wayland_subsurface_get_position(SDL_Window* window, int* x, int* y);
void sdl_wayland_subsurface_hide(SDL_Window* window);
/* Insert immediately above parent; apply sibling Z-order topmost first. */
bool sdl_wayland_subsurface_stack_above_parent(SDL_Window* window);

/* Native Wayland helpers for RAIL client interactive move/resize. */

/* Bind the seat/pointer early (at window creation) or the first move's press serial is missed. */
void sdl_wayland_move_prepare(SDL_Window* window);

/* Begin an interactive move using the latest button-press serial; call while the button is held. */
bool sdl_wayland_begin_move(SDL_Window* window);

/* Begin a compositor-interactive resize; `xdgEdge` is an XDG_TOPLEVEL_RESIZE_EDGE_* value. */
bool sdl_wayland_begin_resize(SDL_Window* window, uint32_t xdgEdge);
