@echo off

IF NOT DEFINED clset (call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64)

set exe_name="GO Game"
set build_options= -DWIN32 -D_WINDOWS 
set compile_flags= /nologo /W3 /Zi /FC /EHsc /TP /Od /Ob0 /MDd /INCREMENTAL
set link_flags=/link raylib.lib user32.lib winmm.lib kernel32.lib opengl32.lib gdi32.lib glfw.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib -opt:ref -incremental:no -NODEFAULTLIB:MSVCRT

set include_dir=/I ../include/  /I ../include/raylib/

set core=../source/go_entry.c

if not exist build mkdir build
pushd build
cl %build_options% %compile_flags% %core% %include_dir% %link_flags% /libpath:..\lib /out:%exe_name%.exe
copy ..\data\* . >NUL
popd