@echo off
set THIS_DIR=%~sdp0

copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug\*.exe    %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug\*.dll    %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug\*.lib    %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug\*.map    %THIS_DIR%windows-vs2013\x86\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug\*.pdb    %THIS_DIR%windows-vs2013\x86\

copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug64x\*.exe %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug64x\*.dll %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug64x\*.lib %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug64x\*.map %THIS_DIR%windows-vs2013\x86_64\
copy /y %THIS_DIR%..\..\build\windows-vs2013\_debug64x\*.pdb %THIS_DIR%windows-vs2013\x86_64\

del %THIS_DIR%windows-vs2013\x86\test_msg_client.map
del %THIS_DIR%windows-vs2013\x86\test_msg_client.pdb
del %THIS_DIR%windows-vs2013\x86\test_rtp.map
del %THIS_DIR%windows-vs2013\x86\test_rtp.pdb
del %THIS_DIR%windows-vs2013\x86\test_tcp_server.map
del %THIS_DIR%windows-vs2013\x86\test_tcp_server.pdb
del %THIS_DIR%windows-vs2013\x86\test_tcp_client.map
del %THIS_DIR%windows-vs2013\x86\test_tcp_client.pdb

del %THIS_DIR%windows-vs2013\x86_64\test_msg_client.map
del %THIS_DIR%windows-vs2013\x86_64\test_msg_client.pdb
del %THIS_DIR%windows-vs2013\x86_64\test_rtp.map
del %THIS_DIR%windows-vs2013\x86_64\test_rtp.pdb
del %THIS_DIR%windows-vs2013\x86_64\test_tcp_server.map
del %THIS_DIR%windows-vs2013\x86_64\test_tcp_server.pdb
del %THIS_DIR%windows-vs2013\x86_64\test_tcp_client.map
del %THIS_DIR%windows-vs2013\x86_64\test_tcp_client.pdb

pause
