![Traymond](https://github.com/dkxce/fcFn.github.io/blob/master/images/logos/traymond_logo.png) Traymond
(mod by dkxce)
=======

A very simple app for minimizing any window to tray as an icon. Runs in the background.

In case it terminates unexpectedly, restart the app and all the icons for minimized windows will come back.

Windows 7 or later required.

**NB**: Does **_NOT_** work with apps from the Microsoft Store (see [#3](/../../issues/3)).

A binary is available [here](https://github.com/dkxce/traymond/releases).

Installing
------------

No installation required, just run Traymond.exe.

Controls (v1.2.6)
--------

+ __Win key + Shift + Z__: Minimize the currently focused window to tray.
+ __Double click on an icon__: Bring back the corresponding hidden window.
+ __Tray icon menu__ accessible by right-clicking the Traymond tray icon:
  + __Click on menu item__: Bring back the corresponding hidden window.
  + __Restore all windows__: Restore all previously hidden windows.
  + __Hide Foreground Window to Tray__: Minimize the current foreground window to tray.
  + __Help (Win+Shift+Z -> Tray)__: Open current page in browser.
  + __Exit__: Exit Traymond and restore all previously hidden windows.

Building
--------

### Nmake

`> nmake`

Please read [this](https://msdn.microsoft.com/en-us/library/f35ctcxw.aspx) if there are any troubles.

Customizing
-------------

Defines at the top of the file control the key and the mod key for sending windows to tray (use virtual key codes from [here](https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx) and mod keys from [here](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646309(v=vs.85).aspx)):
```
#define TRAY_KEY VK_Z_KEY
#define MOD_KEY MOD_WIN + MOD_SHIFT
```
Contributing
------------

See [Contributing](https://github.com/fcFn/traymond/blob/master/CONTRIBUTING.md).
