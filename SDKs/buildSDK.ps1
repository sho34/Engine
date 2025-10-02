$sdkfolder=&"pwd"
$msbuild = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe

#build assimp
cd "assimp\assimp-5.4.3"
cmake.exe .\CMakeLists.txt -G "Visual Studio 17 2022" -BUILD_SHARED_LIBS=OFF -A x64 -B "build"
cd build
& $msbuild ./Assimp.sln /p:Configuration=Release /t:Rebuild
& $msbuild ./Assimp.sln /p:Configuration=Debug /t:Rebuild

#build DirectXTex
cd $sdkfolder
cd DirectXTex
& $msbuild ./DirectXTex_Desktop_2022.sln /p:Configuration=ReleaseLib /t:Rebuild
& $msbuild ./DirectXTex_Desktop_2022.sln /p:Configuration=DebugLib /t:Rebuild

#build DirectXTK12
cd $sdkfolder
cd DirectXTK12
& $msbuild DirectXTK_Desktop_2022_Win10.sln /p:Configuration=Release /t:Rebuild
& $msbuild DirectXTK_Desktop_2022_Win10.sln /p:Configuration=Debug /t:Rebuild

#build imgui
cd $sdkfolder
cd imgui
& $msbuild imgui.sln /p:Configuration=Release /t:Rebuild
& $msbuild imgui.sln /p:Configuration=Debug /t:Rebuild

#build imguizmo
cd $sdkfolder
cd imguizmo
& $msbuild ImGuizmo.sln /p:Configuration=Release /t:Rebuild
& $msbuild ImGuizmo.sln /p:Configuration=Debug /t:Rebuild
