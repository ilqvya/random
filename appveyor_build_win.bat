@echo on

IF "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2015" (
    IF "%PLATFORM%" == "x86" (
        SET GEN="Visual Studio 14 2015"
    ) ELSE (
        SET GEN="Visual Studio 14 2015 Win64"
    )
)
IF "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2017" (
    IF "%PLATFORM%" == "x86" (
        SET GEN="Visual Studio 15 2017"
    ) ELSE (
        SET GEN="Visual Studio 15 2017 Win64"
    )
)
IF "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2019" (
    SET GEN="Visual Studio 16 2019"
    IF "%PLATFORM%" == "x86" (
        SET ARCH=-A Win32
    ) ELSE (
        SET ARCH=-A x64
    )
)
IF "%APPVEYOR_BUILD_WORKER_IMAGE%" == "Visual Studio 2022" (
    SET GEN="Visual Studio 17 2022"
    IF "%PLATFORM%" == "x86" (
        SET ARCH=-A Win32
    ) ELSE (
        SET ARCH=-A x64
    )
)

echo CI_DEBUG_GEN_ARCH: %GEN% %ARCH%
echo CI_DEBUG_CONFIGURATION: %CONFIGURATION%
cmake . -G %GEN% %ARCH% -DCMAKE_CXX_FLAGS="%additional_flags%"
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build . --config %CONFIGURATION%
if %errorlevel% neq 0 exit /b %errorlevel%

ctest -C %CONFIGURATION% --output-on-failure
if %errorlevel% neq 0 exit /b %errorlevel%