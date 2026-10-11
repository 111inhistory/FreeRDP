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
input conversion uses the local parent-relative position instead of the newest
server rect. On niri, button-down also saves that origin until all mouse buttons
are released: niri 26.04's Smithay `ClickGrab` keeps reporting coordinates against
the press-time surface origin, even if the surface moves. Adding a live origin
during that grab would count window movement again. Pixel-density conversion
is unchanged. Other compositors keep the live-origin conversion.

Source references: [niri 26.04 Cargo.lock](https://github.com/niri-wm/niri/blob/v26.04/Cargo.lock)
and [its Smithay ClickGrab implementation](https://github.com/Smithay/smithay/blob/ff5fa7df392cecfba049ffed55cdaa4e98a8e7ef/src/input/pointer/grab.rs).
Retest this compatibility assumption when upgrading niri/Smithay.

The native surface smoke test is not a substitute for these application tests.
