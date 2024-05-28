Traymond II
=======

A simple app for minimizing any window to tray as an icon. Runs in the background.

In case it terminates unexpectedly, restart the app and all the icons for minimized windows will come back.

Windows 7 or later required.

**NB**: Does **_NOT_** work with apps from the Microsoft Store (see [#3](/../../issues/3)).

A binary is available [here](https://github.com/dkxce/traymond/releases).

Installing
------------

No installation required, just run Traymond2.exe.

Controls (v2.0.0)
--------

+ __Win key + Shift + Z__: Minimize the currently focused window to tray.
+ __Double click on an icon__: Bring back the corresponding hidden window.
+ __Tray icon menu__ accessible by right-clicking the Traymond tray icon:
  + __Click on menu item__: Bring back the corresponding hidden window.
  + __Restore all windows__: Restore all previously hidden windows.
  + __Hide Foreground Window to Tray__: Minimize the current foreground window to tray.
  + __Autorun at Windows Startup__: Autorun Traymond at Windows Startup.
  + __Disable Tray MultiIcons__: Disable Tray MultiIcons (Trayed Applications Icons).
  + __Web Site__: Open current page in browser.
  + __Exit__: Exit Traymond and restore all previously hidden windows.

Change Hoy Key:
---------------
To change hot key you must set command line argument `/key=...`, for exmaple:
- traymond2`/key=Shift+Win+X`
- traymond2`/key=Ctrl+Alt+R`
- traymond2`/key=Alt+Shift+F2`
- traymond2`/key=Shift+Alt+Back`
  
Supported combinations:     
- `Alt`+, `Ctrl`+, `Shift`+, `Win`+
- `TAB`,`PAUSE`,`SPACE`,`PRIOR`,`PAGEUP`,`NEXT`,`PAGEDOWN`,`END`,`HOME`,`LEFT`,`UP`,`RIGHT`,`DOWN`,`PRINT`,`INS`,`DEL`,`F10`,`F11`,`F12`,`F1`,`F2`,`F3`,`F4`,`F5`,`F6`,`F7`,`F8`,`F9`,`0`-`9`,`A`-`Z`

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

