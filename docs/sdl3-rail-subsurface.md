# SDL3 RAIL Wayland subsurfaces

Developed on `experiment/sdl3-rail-subsurface` and integrated into
`feat/sdl3-rail-wayland-niri`. The previous `xdg_popup` implementation remains
available at `02342122`; the pre-input-fix baseline is `daaa3260`.

On Wayland builds with native Wayland support, RAIL popups use SDL custom-role
windows with `wl_subsurface`. SDL still owns the buffers, renderer, scaling and
mouse events. Subsurfaces are desynchronized for independent GFX updates;
position and sibling stacking changes explicitly commit the parent. Hiding
attaches a null buffer because SDL hide is otherwise a no-op for custom roles.
Other backends retain SDL popups. Unrelated toplevel stacking is still managed
by the compositor.

Borderless Wayland toplevels explicitly set `xdg_surface.window_geometry` to
`(0, 0, logicalWidth, logicalHeight)` before renderer presentation and parent
commits for subsurface creation/move/restacking. This keeps the WM layout bounds
at the main surface instead of the default union with its subsurfaces. Decorated
windows remain under SDL/libdecor geometry management. This is not a subsurface
rendering or input-region clip; keep the niri clipping rule below.

A native niri 26.04 test at 200% reproduced the default geometry expansion:
a 794x949 main window became a 794x1009 layout with an above-window tooltip,
954x949 with a right-overhanging menu and 914x949 with a left-overhanging popup.
With explicit geometry its layout stayed 794x949 through all three cases,
restacking and hiding; resizing the main window updated it to 794x829.
The protocol trace used logical dimensions, not 1588x1898 buffer pixels.
The reported tooltip jump and left/right column expansion were subsequently
confirmed fixed in application testing. Keep checking popup visibility and clicks
outside the owner's geometry when changing this path.

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

Fullscreen size detection must compare server-pixel window dimensions with display
bounds converted to server pixels. At 200%, an 1810x1200 owned Qt dialog is smaller
than a 1600x1000 logical display (3200x2000 pixels), not a fullscreen popup.
Size alone is also insufficient to choose a role: only parentless, unowned
full-display popups may be promoted. Owned or fallback-parented dialogs retain
their popup role even if an initial window order reports display-sized geometry.
Creation logs include Owner, parent SDL id, GFX dimensions and the fullscreen decision.

A hidden native Wayland lifecycle regression reproduced the old behavior using an
initial 3200x2000 window order followed by actual 1708x1328 geometry: the old window
remained an unparented fullscreen 3200x2000 buffer. With the ownership guard it stays
a native subsurface and resizes to 854x664 logical / 1708x1328 pixels. The same test
checks fallback parenting, waiting for an unavailable owner and preserving the
parentless, unowned full-display path. This is a controlled replay, not a recording
of the application's initial window orders.

`isFullscreenPopup()` describes the creation-time surface role, not the WM's live
fullscreen state. Retest with a new client process: cancelling fullscreen in niri
does not recreate an already-promoted SDL toplevel as a subsurface.

## Maintenance invariants

- Keep native protocol/lifetime handling in `sdl_wayland.cpp`; SDL owns buffers,
  rendering, scaling and input, and `SdlRailWindow` owns RAIL role selection.
- Position/restacking and owner geometry must precede the same parent commit;
  child buffer commits remain independent. Use `commit_subsurface_parent()`.
- Do not replace surface-local logical dimensions with buffer or server pixels.
- Keep press-origin tracking niri-specific, and retain multi-button, release and
  window-deletion cleanup. Test 100%, 150% and 200% pointer conversion.
- Preserve native-support-disabled builds and the SDL popup path on other backends.

Regression coverage: pointer-grab conversion (462 assertions), hidden native
owned/fallback-parented/owner-unavailable/full-display lifecycle replay, native
niri geometry before/after tooltip and menu overhang, main-window resize, and
compilation without native Wayland support. These controlled tests do not replace
application checks for dragging, menu clicks, popup order, hide/reopen and owner close.
