@ECHO OFF
FOR /R %%F IN (*.h,*.cpp) DO (
echo %%~nxF
iconv.exe -f UTF-8 -t GB2312 %%F > %%F.utf8
move %%F.utf8 %%F >nul
)
PAUSE