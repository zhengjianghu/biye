@echo off
REM THIS IS A GENERATED FILE: PLEASE DO NOT EDIT
REM Post Build processing for IAR Workbench.
SET TARGET_BPATH=%*%

echo " "
echo "This converts S37 to Ember Bootload File format if a bootloader has been selected in AppBuilder"
echo " "
@echo on
cmd /C ""%ISA3_UTILS_DIR%\em3xx_convert.exe" $--em3xxConvertFlags--$ "%TARGET_BPATH%.s37" "%TARGET_BPATH%.ebl" > "%TARGET_BPATH%-em3xx-convert-output.txt""
@echo off
type "%TARGET_BPATH%-em3xx-convert-output.txt"
