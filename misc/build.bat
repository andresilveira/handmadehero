@echo off

mkdir ..\..\build
pushd ..\..\build
:: -FC tells the compiler to show full path of files in the error messages
:: -Z[x] are the debug flags
cl -FC -Zi w:\code\win32_handmade.cpp user32.lib gdi32.lib
popd
