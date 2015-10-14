@setlocal
@set TMPEXE=Release\fork-and-pipe.exe
@if NOT EXIST %TMPEXE% goto NOEXE
@set TMP3RD=F:\Projects\software\bin
@if NOT EXIST %TMP3RD%\nul goto no3RD

@set PATH=%TMP3RD%;%PATH%

%TMPEXE%

@goto END

:NOEXE
@echo Can NOT locate %TMPEXE%! *** FIX ME ***
@goto END

:NO3RD
@echo Can NOT locate %TMP3RD%! *** FIX ME ***
@goto END

:END
