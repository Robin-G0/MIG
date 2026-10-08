param([string]$CMakePath = 'cmake', [string]$Sdk = 'build/examples-sdk/windows')
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/../lib/download.ps1"
$projectRoot = [IO.Path]::GetFullPath("$PSScriptRoot/../..")
Set-Location $projectRoot
if (-not (Get-Command $CMakePath -ErrorAction SilentlyContinue)) {
    $CMakePath = 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
}
$dependencies = Join-Path $projectRoot 'build/example-deps'
New-Item -ItemType Directory -Force $dependencies | Out-Null
function Get-ExampleDependency([string]$name, [string]$url, [string]$hash) {
    $archive = Join-Path $dependencies "$name.zip"
    Get-MigArtifact -Uri $url -Destination $archive -Sha256 $hash
    Expand-Archive -LiteralPath $archive -DestinationPath $dependencies -Force
}
Get-ExampleDependency 'sdl' 'https://www.libsdl.org/release/SDL2-devel-2.30.12-VC.zip' '1ca980f9964fb44cf94be235c9818fa1dc8e3f76b08096d751b2d689e5e37d02'
Get-ExampleDependency 'sfml' 'https://github.com/SFML/SFML/releases/download/2.6.2/SFML-2.6.2-windows-vc17-64-bit.zip' 'f5995724604d74d06efe89544315d11ac2d3c2d348854c6be55a7a11fb7ecad8'
Get-ExampleDependency 'sdl-ttf' 'https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.24.0/SDL2_ttf-devel-2.24.0-VC.zip' '2dea8ea01e04756ead27e196a681034ed71342562a75b512ea279fcdfde83307'
$sdkPath = [IO.Path]::GetFullPath((Join-Path $projectRoot $Sdk))
foreach ($example in @('sdl2', 'sfml')) {
    $dependency = if ($example -eq 'sdl2') { 'SDL2-2.30.12' } else { 'SFML-2.6.2' }
    $prefix = "$sdkPath;$(Join-Path $dependencies $dependency);$(Join-Path $dependencies 'SDL2_ttf-2.24.0')"
    $build = "build/examples-compile/windows-$example"
    & $CMakePath -S "examples/$example" -B $build -A x64 "-DCMAKE_PREFIX_PATH=$prefix"
    if ($LASTEXITCODE) { throw "Example configuration failed: $example" }
    & $CMakePath --build $build --config Release
    if ($LASTEXITCODE) { throw "Example compilation failed: $example" }
}
Copy-Item -LiteralPath "$dependencies/SDL2-2.30.12/lib/x64/SDL2.dll" -Destination 'build/examples-compile/windows-sdl2/Release' -Force
Copy-Item -LiteralPath "$dependencies/SDL2_ttf-2.24.0/lib/x64/SDL2_ttf.dll" -Destination 'build/examples-compile/windows-sdl2/Release' -Force
Get-ChildItem -LiteralPath "$dependencies/SFML-2.6.2/bin" -Filter '*.dll' | Where-Object {
    $_.BaseName -in @('sfml-graphics-2', 'sfml-window-2', 'sfml-system-2')
} | Copy-Item -Destination 'build/examples-compile/windows-sfml/Release' -Force
$previousVideoDriver = $env:SDL_VIDEODRIVER
try {
    $env:SDL_VIDEODRIVER = 'dummy'
    & './build/examples-compile/windows-sdl2/Release/mig-sdl2.exe' --smoke
    if ($LASTEXITCODE) { throw 'SDL2 smoke check failed.' }
    & './build/examples-compile/windows-sfml/Release/mig-sfml.exe' --smoke
    if ($LASTEXITCODE) { throw 'SFML smoke check failed.' }
    foreach ($example in @('sdl2', 'sfml')) {
        & "./build/examples-compile/windows-$example/Release/mig-$example-profile.exe" --smoke
        if ($LASTEXITCODE) { throw "Profile smoke check failed: $example" }
    }
} finally { $env:SDL_VIDEODRIVER = $previousVideoDriver }
foreach ($example in @('sdk-consumer', 'native-consumer')) {
    $build = "build/examples-compile/windows-$example"
    & $CMakePath -S "examples/$example" -B $build -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$sdkPath"
    if ($LASTEXITCODE) { throw "Consumer configuration failed: $example" }
    & $CMakePath --build $build --config Release
    if ($LASTEXITCODE) { throw "Consumer compilation failed: $example" }
}
