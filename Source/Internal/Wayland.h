/**
 * @file Wayland.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides Iridium's internal Wayland client
 * implementation. All convoluted Wayland-ness has been confined into
 * simple functions.
 *
 * Copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_INTERNAL_WAYLAND_H
#define IRIDIUM_INTERNAL_WAYLAND_H

typedef struct wl_display *ir_wayland_display_t;
typedef struct wl_registry *ir_wayland_registry_t;

typedef struct wl_compositor *ir_wayland_compositor_t;
typedef struct wl_output *ir_wayland_output_t;
typedef struct xdg_wm_base *ir_wayland_window_manager_t;

bool Ir_WaylandConnect(void);

void Ir_WaylandDisconnect(void);

#endif // IRIDIUM_INTERNAL_WAYLAND_H
