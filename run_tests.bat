@echo off
REM 元のディレクトリを保存
setlocal
set ORIG_DIR=%CD%

REM ビルドディレクトリ作成
if not exist build mkdir build
cd build

REM CMakeでプロジェクト構成
cmake ..
if errorlevel 1 (
    cd "%ORIG_DIR%"
    exit /b 1
)

REM ビルド
cmake --build .
if errorlevel 1 (
    cd "%ORIG_DIR%"
    exit /b 1
)

REM テスト実行
if exist Debug\test_all.exe (
    Debug\test_all.exe -v high
) else if exist test_all.exe (
    test_all.exe -v high
) else (
    echo test_all.exe が見つかりませんでした。
    cd "%ORIG_DIR%"
    exit /b 1
)
cd "%ORIG_DIR%"
endlocal
