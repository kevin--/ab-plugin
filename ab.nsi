!include "LogicLib.nsh"

; The name of the installer
Name "A+/-B Plugin"

; The file to write
OutFile "abplugin.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Winamp

; detect winamp path from uninstall string if available
InstallDirRegKey HKLM \
                 "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
                 "UninstallString"

; The text to prompt the user to enter a directory
DirText "Please select your Winamp path below (you will be able to proceed when Winamp is detected):"

; automatically close the installer when done.
AutoCloseWindow true

; hide the "show details" box
ShowInstDetails nevershow

;--------------------------------

;Pages

Page components
Page directory
Page instfiles

;--------------------------------

; The stuff to install
Section "!A+/-B Plugin"

StartFind:
	FindWindow $0 "Winamp v1.x"
	${If} $0 != 0
		MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Winamp is currently running. Please close it, and click Retry." IDRETRY StartFind IDCANCEL Die
	${Else}
		Goto WinampOff
	${EndIf}
Die:
	Quit
WinampOff:
	

	Call QueryWinampDSPPath
	SetOutPath $1

	File "E:\proggy\projects\ab plugin\dsp_ab.dll"

	MessageBox MB_YESNO|MB_ICONINFORMATION "A+/-B Analysis Tool was successfully installed. Once Winamp is running, enter the Preferences and select DSP/Effect. You'll find the plugin in the list. Would you like to start Winamp now?" IDNO Done

	Exec "$INSTDIR\Winamp.exe"

Done:
  
SectionEnd

Section "Source Code"
	CreateDirectory "$INSTDIR\AB Plugin Source"
	SetOutPath "$INSTDIR\AB Plugin Source"
	File "E:\proggy\projects\ab plugin\resource.h"
	File "E:\proggy\projects\ab plugin\dsp.h"
	File "E:\proggy\projects\ab plugin\main.cpp"
	File "E:\proggy\projects\ab plugin\dialog.rc"
 	File "E:\proggy\projects\ab plugin\install\ab.nsi"
	File "E:\proggy\projects\ab plugin\readme.txt"
SectionEnd


;--------------------------------

Function .onVerifyInstDir

!ifndef WINAMP_AUTOINSTALL

  ;Check for Winamp installation

  IfFileExists $INSTDIR\Winamp.exe Good
    Abort
  Good:

!endif ; WINAMP_AUTOINSTALL

FunctionEnd

Function QueryWinampDSPPath ; sets $1 with dsp path

  StrCpy $1 $INSTDIR\Plugins
  ; use DSPDir instead of VISDir to get DSP plugins directory
  ReadINIStr $9 $INSTDIR\winamp.ini Winamp DSPDir 
  StrCmp $9 "" End
  IfFileExists $9 0 End
    StrCpy $1 $9 ; update dir
  End:
  
FunctionEnd