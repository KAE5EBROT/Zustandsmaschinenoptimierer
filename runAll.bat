@echo off
echo Demos:
echo     1 kleine_automatentabelle.txt
echo     2 mittlere_automatentabelle.txt
echo     3 grosse_automatentabelle.txt
echo     4 riesige_automatentabelle.txt
set /p inputfc=Choose demo number or enter file:
if %inputfc%==1 set inputfc=.\kleine_automatentabelle.txt
if %inputfc%==2 set inputfc=.\mittlere_automatentabelle.txt
if %inputfc%==3 set inputfc=.\grosse_automatentabelle.txt
if %inputfc%==4 set inputfc=.\riesige_automatentabelle.txt
if exist %inputfc% (
	echo running optimizer for %inputfc%...
	.\Debug\Zustandsmaschinencodierungsoptimierer.exe %inputfc%
	echo running minilog...
	cscript runMinilog.vbs
	echo running gate counter...
	"gatter.exe - Verknuepfung.lnk"
) else (
	echo ERROR: File not found!
)
pause