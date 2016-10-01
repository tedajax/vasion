cl vasion.c /I C:\dev\include /c /Z7
link SDL2.lib SDL2main.lib vasion.obj /MANIFEST /LIBPATH:C:\dev\lib\x64 /SUBSYSTEM:console  /out:vasion.exe /debug
