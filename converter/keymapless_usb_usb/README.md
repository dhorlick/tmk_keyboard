Keymapless USB protocol converter
=================================
A converter for remapping functions of USB devices without the use of keymaps.

This includes HID devices like:

   * USB mice and mouse-like USB devices
   * USB MIDI controllers
   * USB keyboards & keypads
   
See "USB to USB keyboard protocol converter" README for requirements, most build details and resources. Keymaps aren't supported, so ignore that part.

Specify a REMIX variable when compiling. There are scrollball, arrowkeys, naptime, midimacropad, knob and mousepassthru remixes.

```bash
make dfu REMIX=scrollball
```

```bash
make dfu REMIX=arrowkeys
```

```bash
make dfu REMIX=naptime
```

```bash
make dfu REMIX=midimacropad
```

```bash
make dfu REMIX=knob
```

```bash
make dfu REMIX=mousepassthru
```

New remix source files must be implemented in C++. Their names should be prefixed with "remix_" and suffixed with ".cpp".

### Troubleshoooting

Mice communicate using boot or report protocol. You can request one or the other via the second, boolean argument to the HIDBoot constructor.

Most mice support boot protocol. This protocol is well-standardized, but it typically does not include scroll information.

Some mice support report protocol. This generally conveys scroll information when implemented, but varies radically from device to device.

If your pointer seems to be much less sensitive on one axis, chances are that it is communicating over a report protocol when this converter has been configured for boot protocol.

If your pointer, clicking or scrolling goes crazy, then the opposite may be true.

Try hid_listen (see tmk_core/README.md) for more information