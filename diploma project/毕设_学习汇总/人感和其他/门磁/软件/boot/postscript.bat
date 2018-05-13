cd "..\..\build\boot-cortexm3-iarboot-em357-null-dev0680"
rm -rf boot.*
..\..\tools\bin\otaconv.exe "boot=..\..\build\boot-cortexm3-iarboot-em357-null-dev0680\boot.hex"
