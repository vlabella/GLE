@echo off
setlocal enabledelayedexpansion
:: GLE Installation Script for Windows
:: Default installation directory
DEFAULT_INSTALL_PREFIX=%ProgramFiles%\gle-graphics
ENV_KEY=HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment
echo "\e[31mG\e[32mL\e[34mE\e[0m Installation Script"

set INSTALL_DIR=
set CREATE_FILETYPE_ASSOCIATION=1
set INSTALL=1
set UNINSTALL=0
set ADMIN_NEEDED=1
set PREFIX_NEEDED=1
set MY_EDITOR=

:: Parse command-line arguments
:parse_args
if "%~1"=="" goto arg_parse_done

if "%~1"=="--prefix" (
    shift
    set INSTALL_DIR=%~1
    shift
    goto parse_args
) else if "%~1"=="--editor" (
    shift
    set MY_EDITOR=%~1
    shift
    goto parse_args
) else if "%~1"=="--create_filetype_association" (
    set CREATE_FILETYPE_ASSOCIATION=1
    set INSTALL=0
    set ADMIN_NEEDED=0
    set PREFIX_NEEDED=0
    shift
    goto parse_args
) else if "%~1"=="--no_create_filetype_association" (
    set CREATE_FILETYPE_ASSOCIATION=0
    shift
    goto parse_args
) else if "%~1"=="--uninstall" (
    set UNINSTALL=1
    set CREATE_FILETYPE_ASSOCIATION=0
    shift
    goto parse_args
) else (
    echo Usage: %~nx0 [--prefix "C:\Program Files"] [--create_filetype_association] [--uninstall]
    exit /b 1
)
:arg_parse_done

if "%ADMIN_NEEDED%" == "1" (
    :: Check for Administrator privileges
    net session >nul 2>&1
    if %errorlevel% neq 0 (
        echo Error: Please run this script as Administrator.
        exit /b 1
    )
)

if "%PREFIX_NEEDED%" == "1" (
    :: Prompt user if no prefix provided
    set /p USER_INPUT=Choose installation directory [%DEFAULT_INSTALL_PREFIX% by default]:
    if "%USER_INPUT%"=="" (
        set INSTALL_DIR=%DEFAULT_INSTALL_PREFIX%
    ) else (
        set INSTALL_DIR=%USER_INPUT%
    )
)

if "%UNINSTALL%" == "1" (
    echo Removing GLE from %INSTALL_DIR%...
    :: Create directories
    rmdir /S "%INSTALL_DIR%"
    :: clean registry
    REG DELETE HKCR\.gle /f
    REG DELETE HKCR\glefile /f
    :: delete GLE_TOP variable
    set GLE_TOP=
    REG DELETE "%ENV_KEY%" /v GLE_TOP /f
    :: remove from PATH_EXT
    set "RegPath=%ENV_KEY"
    set "ValueName=PATHEXT"
    set "AppendText=;.GLE"
    for /f "tokens=3*" %%A in ('reg query "%RegPath%" /v "%ValueName%" ^| find "%ValueName%"') do (
        set "CurrentValue=%%A %%B"
    )
    :: Check if CurrentValue is empty
    if "%CurrentValue%"=="" (
        echo Value not found or empty. Nothing to remove.
        goto :EOF
    )
    :: echo Current value: %CurrentValue%
    :: Remove the specified text
    set "NewValue=%CurrentValue%"
    set "NewValue=!NewValue:%RemoveText%=!"
    :: Write updated value back to registry
    reg add "%RegPath%" /v "%ValueName%" /t REG_SZ /d "%NewValue%" /f

    :: remove from PATH
    set "RegPath=%ENV_KEY"
    set "ValueName=PATH"
    set "AppendText=;%INSTALL_DIR%\bin"
    for /f "tokens=3*" %%A in ('reg query "%RegPath%" /v "%ValueName%" ^| find "%ValueName%"') do (
        set "CurrentValue=%%A %%B"
    )
    :: Check if CurrentValue is empty
    if "%CurrentValue%"=="" (
        echo Value not found or empty. Nothing to remove.
        goto :EOF
    )
    :: echo Current value: %CurrentValue%
    :: Remove the specified text
    set "NewValue=%CurrentValue%"
    set "NewValue=!NewValue:%RemoveText%=!"
    :: Write updated value back to registry
    reg add "%RegPath%" /v "%ValueName%" /t REG_SZ /d "%NewValue%" /f
)

if "%INSTALL%" == "1" (
    echo Installing to %INSTALL_DIR%...
    :: Create directories
    mkdir "%INSTALL_DIR%"
    mkdir "%INSTALL_DIR%\bin"
    mkdir "%INSTALL_DIR%\fonts"
    mkdir "%INSTALL_DIR%\doc"
    mkdir "%INSTALL_DIR%\gleinc"
    :: Copy files (assuming script is in root of GLE source)
    copy bin\gle.exe "%INSTALL_DIR%\bin\"
    copy bin\qgle.exe "%INSTALL_DIR%\bin\"
    :: copy bin\manip.exe "%INSTALL_DIR%\bin\" (linux only)
    copy glerc "%INSTALL_DIR%"
    copy init.tex "%INSTALL_DIR%"
    copy initex.ini "%INSTALL_DIR%"
    copy doc\* "%INSTALL_DIR%\doc"
    copy gleinc\*.txt "%INSTALL_DIR%\doc"
    copy gleinc\*.gle "%INSTALL_DIR%\gleinc"
    xcopy font "%INSTALL_DIR%\font" /E /I /Y
    :: Set GLE_TOP variable
    set GLE_TOP=%INSTALL_DIR%\share\gle-graphics
    REG ADD "%ENV_KEY%" /f /v GLE_TOP /t REG_EXPAND_SZ /d "%INSTALL_DIR"
)

if %CREATE_FILETYPE_ASSOCIATION% == "1" (
    REG ADD HKCR\.gle /f /ve /d glefile
    REG ADD HKCR\.gle /f /v "Content Type" /t REG_SZ /d application/gle
    REG ADD HKCR\.gle /f /v "PerceivedType" /t REG_SZ /d text
    REG ADD HKCR\glefile /f /ve /d "GLE Script"
    REG ADD HKCR\glefile /f /v EditFlags /t REG_DWORD /d 0
    REG ADD HKCR\glefile /f /v BrowserFlags /t REG_DWORD /d 8
    REG ADD HKCR\glefile /f /v AlwaysShowExt /t REG_SZ
    REG ADD HKCR\glefile\DefaultIcon /f /ve /d "%INSTALL_DIR%\bin\qgle.exe,0"
    REG ADD HKCR\glefile\shell\open\command /f /v " " /d "\"%INSTALL_DIR%\bin\qgle.exe\" -f %%1" /t REG_SZ
    reg add HKCR\glefile\shell\edit /v MUIVerb /d "Edit" /f
    REG ADD HKCR\glefile\shell\edit\command /f /v " " /d "\"%MY_EDITOR%\" %%1" /t REG_SZ
    :: add to PATH if not allready present
    set "ValueName=PATH"
    set "AppendText=;%INSTALL_DIR%\bin"
    :: Query current value
    for /f "tokens=3*" %%A in ('reg query "%RegPath%" /v "%ValueName%" ^| find "%ValueName%"') do (
        set "CurrentValue=%%A %%B"
    )
    :: Check if CurrentValue is empty
    if "%CurrentValue%"=="" (
        echo Value not found or empty. Creating new value.
        set "NewValue=%AppendText%"
    ) else (
        echo Current value: %CurrentValue%
        set "NewValue=%CurrentValue%%AppendText%"
    )
    :: Write updated value back to registry
    reg add "%RegPath%" /v "%ValueName%" /t REG_SZ /d "%NewValue%" /f
    :: add to PATHEXT if not allready present
    :: Define registry path, value name, and text to append
    set "RegPath=%ENV_KEY"
    set "ValueName=PATHEXT"
    set "AppendText=;.GLE"
    :: Query current value
    for /f "tokens=3*" %%A in ('reg query "%RegPath%" /v "%ValueName%" ^| find "%ValueName%"') do (
        set "CurrentValue=%%A %%B"
    )
    :: Check if CurrentValue is empty
    if "%CurrentValue%"=="" (
        echo Value not found or empty. Creating new value.
        set "NewValue=%AppendText%"
    ) else (
        echo Current value: %CurrentValue%
        set "NewValue=%CurrentValue%%AppendText%"
    )
    :: Write updated value back to registry
    reg add "%RegPath%" /v "%ValueName%" /t REG_SZ /d "%NewValue%" /f

)
echo GLE Installation complete.