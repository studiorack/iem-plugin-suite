@echo off
set /A compileWin32=1
set /A compilex64=1
mkdir "_compiledPlugins\win32\IEM"
mkdir "_compiledPlugins\x64\IEM"
FOR /D %%G IN (*) do ( 
	IF EXIST "%%G/Builds/VisualStudio2017/%%G.sln" (
		echo "Solution file found for: %%G%
		IF "%compileWin32%" == "1" (
			echo "Compiling for Win32..."
			msbuild "%%G/Builds/VisualStudio2017/%%G.sln" /p:Configuration="Release 32bit",Platform=Win32 /t:Clean,Build
			COPY /Y "%%G\Builds\VisualStudio2017\Win32\Release 32Bit\VST\%%G_Win32.dll" "_compiledPlugins\win32\IEM"
		)

		IF "%compilex64%" == "1" (
			echo "Compiling for x64..."
			msbuild "%%G/Builds/VisualStudio2017/%%G.sln" /p:Configuration="Release 64bit",Platform=x64 /t:Clean,Build
			COPY /Y "%%G\Builds\VisualStudio2017\x64\Release 64Bit\VST\%%G_x64.dll" "_compiledPlugins\x64\IEM"
		)

	)
)

