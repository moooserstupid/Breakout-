Set-Location -Path F:\Code\C\Breakout
gcc main.c -o brkout.exe -C -Wall -std=c99 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
& "F:\Code\C\Breakout\brkout.exe" /run /exit
Start-Sleep -s 3