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
