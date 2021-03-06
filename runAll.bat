@REM (enter commands silently)
@echo off
@REM print examples
echo Demos:
echo     1 kleine_automatentabelle.txt
echo     2 mittlere_automatentabelle.txt
echo     3 grosse_automatentabelle.txt
echo     4 riesige_automatentabelle.txt
@REM request input
set /p inputfc=Choose demo number or enter file:
@REM replace selection
if %inputfc%==1 set inputfc=.\kleine_automatentabelle.txt
if %inputfc%==2 set inputfc=.\mittlere_automatentabelle.txt
if %inputfc%==3 set inputfc=.\grosse_automatentabelle.txt
if %inputfc%==4 set inputfc=.\riesige_automatentabelle.txt
@REM check for existence of file
if exist %inputfc% (
	echo running optimizer for %inputfc%...
	@REM pass file to optimizer
	.\Zustandsmaschinencodierungsoptimierer.exe %inputfc% && (
		@REM this section runs only if exe returns success
		echo running minilog...
		@REM start optimizer script
		cscript runMinilog.vbs
		echo running gate counter...
		@REM run gate counter
		.\gatter.exe
	)
) else (
	echo ERROR: File not found! "%inputfc%"
)
pause