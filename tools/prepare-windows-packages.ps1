$ErrorActionPreference = 'Stop'
Set-Location "$PSScriptRoot/.."
$version = (Get-Content VERSION -Raw).Trim()
function Invoke-Checked([string]$Command, [string[]]$Arguments) {
    & $Command @Arguments
    if ($LASTEXITCODE) { throw "$Command failed with exit code $LASTEXITCODE" }
}
Invoke-Checked cmake @('-S', '.', '-B', 'build/release-sdk-windows', '-A', 'x64',
    '-DMIG_BUILD_CONTROLLER=OFF', '-DMIG_BUILD_CONFIGURATOR=OFF', '-DMIG_BUILD_NATIVE_RUNTIME=OFF')
Invoke-Checked cmake @('--build', 'build/release-sdk-windows', '--config', 'Release', '--parallel', '3')
Invoke-Checked ctest @('--test-dir', 'build/release-sdk-windows', '-C', 'Release', '--output-on-failure')
$sdk = "$PWD/build/release-sdk-windows-install"
Invoke-Checked cmake @('--install', 'build/release-sdk-windows', '--config', 'Release', '--prefix', $sdk)
Invoke-Checked python @('tools/package-sdk.py', '--sdk', $sdk, '--dependencies', 'build/native-deps', '--platform', 'windows-x64')
Invoke-Checked python @('tests/packaging/sdk_package_tests.py', "build/releases/motion-input-grid-$version-windows-x64-sdk.zip")
Invoke-Checked python @('-m', 'pip', 'install', 'build', 'twine')
Invoke-Checked python @('tools/package-python.py', '--destination', 'build/python-windows')
Copy-Item -LiteralPath "build/python-windows/motion_input_grid-$version-py3-none-win_amd64.whl" -Destination build/releases
Invoke-Checked python @('tests/packaging/python_package_tests.py', "build/releases/motion_input_grid-$version-py3-none-win_amd64.whl")
Invoke-Checked python @('-m', 'twine', 'check', "build/releases/motion_input_grid-$version-py3-none-win_amd64.whl", "build/python-windows/motion_input_grid-$version.tar.gz")
foreach ($ecosystem in @('unity', 'unreal')) {
    Invoke-Checked python @('tools/package-integrations.py', '--sdk', $sdk, '--dependencies', 'build/native-deps', '--platform', 'windows-x64', '--ecosystem', $ecosystem)
    $extension = if ($ecosystem -eq 'unity') { 'tgz' } else { 'zip' }
    Invoke-Checked python @('tests/packaging/native_integration_package_tests.py', "build/releases/motion-input-grid-$version-windows-x64-$ecosystem.$extension")
}
Invoke-Checked python @('tools/bootstrap-godot.py', '--editor')
Invoke-Checked cmake @('-S', 'integrations/godot/native', '-B', 'build/release-godot-windows', '-A', 'x64',
    "-DGODOT_CPP_DIR=$PWD/build/godot-deps/godot-cpp-godot-4.3-stable", "-DCMAKE_PREFIX_PATH=$sdk")
Invoke-Checked cmake @('--build', 'build/release-godot-windows', '--config', 'Release', '--parallel', '3')
Invoke-Checked python @('tools/package-integrations.py', '--ecosystem', 'godot', '--sdk', $sdk,
    '--dependencies', 'build/native-deps', '--platform', 'windows-x64',
    '--godot-bridge', 'build/release-godot-windows/Release/mig-godot.dll',
    '--godot-cpp', 'build/godot-deps/godot-cpp-godot-4.3-stable')
Invoke-Checked python @('tests/packaging/godot_package_tests.py', "build/releases/motion-input-grid-$version-windows-x64-godot.zip",
    '--godot', 'build/godot-deps/Godot_v4.3-stable_win64_console.exe')
