# PixelWin

PixelWin is a [Pixelflut server](https://github.com/defnull/pixelflut) written in C for Windows.
It only relies on native Windows APIs, is fairly simple, quick and minimal. I have planned to microoptimize it a bit further in the future to make it even faster, but speed is not the top priority of this project and it still just runs on the CPU with a basic (still fast) algorithm.


# Compiling
To compile PixelWin, you need [mingw64](https://github.com/niXman/mingw-builds-binaries/releases), specifically gcc. To compile in release or debug mode, simply use the corresponding batch scripts [BuildRelease.bat](src/BuildRelease.bat) and [BuildDebug.bat](src/BuildDebug.bat).

