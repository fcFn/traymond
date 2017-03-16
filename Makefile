CPPFLAGS=/nologo /EHsc /O2 /GL /GS /Oi /MD
RFLAGS=/nologo /n /r

{src/}.cpp.obj:
	$(CPP) $(CPPFLAGS) /c $<
{src/}.rc.res:
	$(RC) $(RFLAGS) /fo$(@F) $<

Traymond.exe: traymond.obj Traymond.res
	$(CPP) $(CPPFLAGS) /Fe$(@F) $** user32.lib shell32.lib /link /MACHINE:X86

clean:
	del *.obj *.res Traymond.exe
