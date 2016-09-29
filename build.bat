cl vasion.c /I C:\dev\include /c
link SDL2.lib SDL2main.lib vasion.obj /LIBPATH:C:\dev\lib\x64 /SUBSYSTEM:console  /out:vasion.exe
