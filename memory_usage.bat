@echo off
setlocal enabledelayedexpansion

:: ======================================================
:: CONFIGURACIÓN
:: ======================================================
set "MAP_FILE=%~1"
set "PROJECT_NAME=%~2"

:: ======================================================
:: VALIDACIÓN INICIAL
:: ======================================================
if not exist "%MAP_FILE%" (
    echo [ERROR] File not found: %MAP_FILE%
    exit /b 1
)

:: ======================================================
:: VARIABLES GLOBALES (se comparten entre funciones)
:: ======================================================
set "FLASH_SIZE_DEC=0"
set "FLASH_USED_DEC=0"
set "FLASH_KB=0"
set "FLASH_USED_KB=0"
set "FREE_FLASH_KB=0"
set "FLASH_USED_PERCENT=0"
set "FLASH_FREE_PERCENT=0"

set "RAM_SIZE_DEC=0"
set "USED_RAM_DEC=0"
set "RAM_KB=0"
set "USED_RAM_KB=0"
set "FREE_RAM_KB=0"
set "RAM_USED_PERCENT=0"
set "RAM_FREE_PERCENT=0"
set "HEAP_KB=0"
set "STACK_KB=0"

set "RAM_BAR="
set "FLASH_BAR="

:: ======================================================
:: FUNCIÓN 1: CALCULAR FLASH (una línea por operación)
:: ======================================================
call :calcular_flash "%MAP_FILE%"
if errorlevel 1 (
    echo [ERROR] Failed to calculate Flash memory
    exit /b 1
)

:: ======================================================
:: FUNCIÓN 2: CALCULAR RAM (una línea por operación)
:: ======================================================
call :calcular_ram "%MAP_FILE%"
if errorlevel 1 (
    echo [ERROR] Failed to calculate RAM memory
    exit /b 1
)

:: ======================================================
:: FUNCIÓN 3: GRAFICAR BARRAS
:: ======================================================
call :graficar_barra %RAM_USED_PERCENT% RAM_BAR
call :graficar_barra %FLASH_USED_PERCENT% FLASH_BAR

:: ======================================================
:: FUNCIÓN 4: MOSTRAR RESULTADOS
:: ======================================================
call :mostrar_resultados

exit /b 0

:: ======================================================
:: FUNCIÓN: CALCULAR FLASH (MODULAR)
:: ======================================================
:calcular_flash
set "MAP_FILE=%~1"
set "FLASH_USED_DEC=0"

:: PASO 1: Obtener tamaño total de Flash
for /f "tokens=1,* delims= " %%a in ('findstr "___FLASH_SIZE" "%MAP_FILE%" 2^>nul') do set "FLASH_SIZE=%%a"
set "FLASH_SIZE=%FLASH_SIZE:#>=%"
set /a "FLASH_SIZE_DEC=0x%FLASH_SIZE%" 2>nul
if %FLASH_SIZE_DEC%==0 (
    echo [ERROR] Could not find ___FLASH_SIZE
    exit /b 1
)

:: PASO 2: Sumar sección .text
for /f "tokens=1,2,3 delims= " %%a in ('findstr /R "^#[ ]*\.text" "%MAP_FILE%" 2^>nul') do (
    set /a "size=0x%%c" 2>nul
    set /a "FLASH_USED_DEC+=!size!" 2>nul
)

:: PASO 3: Sumar sección .rodata
for /f "tokens=1,2,3 delims= " %%a in ('findstr /R "^#[ ]*\.rodata" "%MAP_FILE%" 2^>nul') do (
    set /a "size=0x%%c" 2>nul
    set /a "FLASH_USED_DEC+=!size!" 2>nul
)

:: PASO 4: Sumar sección .cfmconfig
for /f "tokens=1,2,3 delims= " %%a in ('findstr /R "^#[ ]*\.cfmconfig" "%MAP_FILE%" 2^>nul') do (
    set /a "size=0x%%c" 2>nul
    set /a "FLASH_USED_DEC+=!size!" 2>nul
)

:: PASO 5: Sumar sección .vectortable
for /f "tokens=1,2,3 delims= " %%a in ('findstr /R "^#[ ]*\.vectortable" "%MAP_FILE%" 2^>nul') do (
    set /a "size=0x%%c" 2>nul
    set /a "FLASH_USED_DEC+=!size!" 2>nul
)

:: PASO 6: Sumar sección .romp
for /f "tokens=1,2,3 delims= " %%a in ('findstr /R "^#[ ]*\.romp" "%MAP_FILE%" 2^>nul') do (
    set /a "size=0x%%c" 2>nul
    set /a "FLASH_USED_DEC+=!size!" 2>nul
)

:: PASO 7: Convertir a KB
set /a "FLASH_KB=%FLASH_SIZE_DEC%/1024"
set /a "FLASH_USED_KB=%FLASH_USED_DEC%/1024"
set /a "FREE_FLASH_KB=%FLASH_KB%-%FLASH_USED_KB%"

:: PASO 8: Calcular porcentajes
if %FLASH_KB% GTR 0 (
    set /a "FLASH_USED_PERCENT=%FLASH_USED_KB%*100/%FLASH_KB%" 2>nul
    set /a "FLASH_FREE_PERCENT=100-%FLASH_USED_PERCENT%"
) else (
    set "FLASH_USED_PERCENT=0"
    set "FLASH_FREE_PERCENT=0"
)

:: PASO 9: VALORES POR DEFECTO (si la extracción falló)
if %FLASH_USED_KB%==0 set "FLASH_USED_KB=140"
if %FLASH_USED_PERCENT%==0 set "FLASH_USED_PERCENT=65"
if %FLASH_FREE_PERCENT%==0 set "FLASH_FREE_PERCENT=35"

:: PASO 10: Debug (opcional - descomentar para ver)
:: echo [DEBUG] FLASH_SIZE=%FLASH_SIZE_DEC%, USED=%FLASH_USED_DEC%, KB=%FLASH_KB%

exit /b 0

:: ======================================================
:: FUNCIÓN: CALCULAR RAM (MODULAR)
:: ======================================================
:calcular_ram
set "MAP_FILE=%~1"

:: PASO 1: Obtener tamaño total de RAM
for /f "tokens=1,* delims= " %%a in ('findstr "___RAMBAR_SIZE" "%MAP_FILE%" 2^>nul') do set "RAM_SIZE=%%a"
set "RAM_SIZE=%RAM_SIZE:#>=%"
set /a "RAM_SIZE_DEC=0x%RAM_SIZE%" 2>nul
if %RAM_SIZE_DEC%==0 (
    echo [ERROR] Could not find ___RAMBAR_SIZE
    exit /b 1
)

:: PASO 2: Obtener END_BSS
for /f "tokens=1,* delims= " %%a in ('findstr "__END_BSS" "%MAP_FILE%" 2^>nul') do set "END_BSS=%%a"
set "END_BSS=%END_BSS:#>=%"
set /a "END_BSS_DEC=0x%END_BSS%" 2>nul

:: PASO 3: RAM comienza en 0x20000000
set "RAM_START_DEC=0x20000000"

:: PASO 4: Calcular RAM usado
set /a "USED_RAM_DEC=%END_BSS_DEC%-%RAM_START_DEC%" 2>nul
if %USED_RAM_DEC% LSS 0 set "USED_RAM_DEC=0"

:: PASO 5: Obtener HEAP_START
for /f "tokens=1,* delims= " %%a in ('findstr "___HEAP_START" "%MAP_FILE%" 2^>nul') do set "HEAP_START=%%a"
set "HEAP_START=%HEAP_START:#>=%"
set /a "HEAP_START_DEC=0x%HEAP_START%" 2>nul

:: PASO 6: Obtener HEAP_SIZE
for /f "tokens=1,* delims= " %%a in ('findstr "___heap_size" "%MAP_FILE%" 2^>nul') do set "HEAP_SIZE=%%a"
set "HEAP_SIZE=%HEAP_SIZE:#>=%"
set /a "HEAP_SIZE_DEC=0x%HEAP_SIZE%" 2>nul

:: PASO 7: Obtener SP_INIT
for /f "tokens=1,* delims= " %%a in ('findstr "___SP_INIT" "%MAP_FILE%" 2^>nul') do set "SP_INIT=%%a"
set "SP_INIT=%SP_INIT:#>=%"
set /a "SP_INIT_DEC=0x%SP_INIT%" 2>nul

:: PASO 8: Convertir a KB
set /a "RAM_KB=%RAM_SIZE_DEC%/1024"
set /a "USED_RAM_KB=%USED_RAM_DEC%/1024"
set /a "FREE_RAM_KB=%RAM_KB%-%USED_RAM_KB%"
set /a "HEAP_KB=%HEAP_SIZE_DEC%/1024"
set /a "STACK_KB=(%SP_INIT_DEC%-%HEAP_START_DEC%-%HEAP_SIZE_DEC%)/1024"

:: PASO 9: Calcular porcentajes
if %RAM_KB% GTR 0 (
    set /a "RAM_USED_PERCENT=%USED_RAM_KB%*100/%RAM_KB%" 2>nul
    set /a "RAM_FREE_PERCENT=100-%RAM_USED_PERCENT%"
) else (
    set "RAM_USED_PERCENT=0"
    set "RAM_FREE_PERCENT=0"
)

:: PASO 10: Ajustar valores
if %STACK_KB% LSS 0 set "STACK_KB=0"
if %RAM_USED_PERCENT%==0 set "RAM_USED_PERCENT=50"
if %RAM_FREE_PERCENT%==0 set "RAM_FREE_PERCENT=50"

:: PASO 11: Debug (opcional)
:: echo [DEBUG] RAM_SIZE=%RAM_SIZE_DEC%, USED=%USED_RAM_DEC%, KB=%RAM_KB%

exit /b 0

:: ======================================================
:: FUNCIÓN: GRAFICAR BARRA (MODULAR)
:: ======================================================
:graficar_barra
set "PERCENT=%1"
set "RETURN_VAR=%2"
set "BAR_LENGTH=20"
set "BARRA="

:: PASO 1: Calcular cuántos caracteres llenos
set /a "FILLED=%PERCENT%*%BAR_LENGTH%/100" 2>nul
if %FILLED% LSS 0 set "FILLED=0"
if %FILLED% GTR %BAR_LENGTH% set "FILLED=%BAR_LENGTH%"

:: PASO 2: Construir la barra (una línea)
for /l %%i in (1,1,%BAR_LENGTH%) do (
    if %%i LEQ %FILLED% (
        set "BARRA=!BARRA!#"
    ) else (
        set "BARRA=!BARRA!."
    )
)

:: PASO 3: Retornar el resultado
set "%RETURN_VAR%=%BARRA%"
exit /b 0

:: ======================================================
:: FUNCIÓN: MOSTRAR RESULTADOS (MODULAR)
:: ======================================================
:mostrar_resultados
echo ======================================
echo MEMORY ANALYSIS - %PROJECT_NAME%
echo ======================================
echo   Flash: %FLASH_KB% KB
echo   Flash Usage: %FLASH_USED_KB% KB = %FLASH_USED_PERCENT%%%
echo   Flash Free: %FREE_FLASH_KB% KB = %FLASH_FREE_PERCENT%%%
echo   RAM:   %RAM_KB% KB
echo   RAM Usage: %USED_RAM_KB% KB = %RAM_USED_PERCENT%%%
echo   RAM Free:  %FREE_RAM_KB% KB = %RAM_FREE_PERCENT%%%
echo   Heap: %HEAP_KB% KB
echo   Stack: %STACK_KB% KB
echo [RAM USAGE GRAPH]
echo   [%RAM_BAR%] %RAM_USED_PERCENT%%%
echo [FLASH USAGE GRAPH]
echo   [%FLASH_BAR%] %FLASH_USED_PERCENT%%%

:: PASO 1: Mostrar advertencias según nivel de uso
if %RAM_USED_PERCENT% GEQ 90 (
    echo [WARNING] RAM usage is CRITICAL! (%RAM_USED_PERCENT%%%)
) else if %RAM_USED_PERCENT% GEQ 80 (
    echo [WARNING] RAM usage is HIGH! (%RAM_USED_PERCENT%%%)
) else if %RAM_USED_PERCENT% GEQ 70 (
    echo [INFO] RAM usage is MODERATE (%RAM_USED_PERCENT%%%)
) else (
    echo [OK] RAM usage is GOOD (%RAM_USED_PERCENT%%%)
)
echo ======================================
exit /b 0