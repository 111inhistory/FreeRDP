# SDL3 RAIL subsurface experiment

Branch: `experiment/sdl3-rail-subsurface`.
The existing `xdg_popup` implementation remains on
`feat/sdl3-rail-wayland-niri` (`02342122`). The pre-input-fix version is
`daaa3260`.

On Wayland builds with native Wayland support, RAIL popups use SDL custom-role
windows with `wl_subsurface`. SDL still owns the buffers, renderer, scaling and
mouse events. Subsurfaces are desynchronized for independent GFX updates;
position and sibling stacking changes explicitly commit the parent. Hiding
attaches a null buffer because SDL hide is otherwise a no-op for custom roles.
Other backends retain SDL popups. Unrelated toplevel stacking is still managed
by the compositor.

Place this niri rule **after** global/DMS rules:

```kdl
window-rule {
    match app-id=r#"^com\.freerdp\.client\.sdl3$"#
    clip-to-geometry false
}
```

Validate with `niri validate`. Start a new client process to test the new binary;
an already running client is unchanged by rebuilding.

Check XtItClient settings/data-management visibility, dragging at 200% scale,
menus/tooltips, overlapping popup order, hide/reopen and owner close. The popup
input fix uses the local parent-relative position instead of the newest server
rect, preventing pending server geometry from being added to pointer movement.
The native surface smoke test is not a substitute for these application tests.
