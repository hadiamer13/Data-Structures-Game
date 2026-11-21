$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$releaseDir = Join-Path $repoRoot 'DS-Project\x64\Release'
$libDir = Join-Path $repoRoot 'lib'
$runtimeDir = Join-Path $libDir 'bin'
$vsRoot = Join-Path $env:ProgramFiles 'Microsoft Visual Studio\18\Community'
$vcvars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat'

if (-not (Test-Path $vcvars)) {
    throw "Visual Studio C++ tools were not found at $vcvars"
}

New-Item -ItemType Directory -Force -Path $releaseDir | Out-Null

$sourceFiles = @(
    'model\AVLTree.cpp',
    'model\BinaryTree.cpp',
    'model\BST.cpp',
    'model\EventLog.cpp',
    'model\MaxHeap.cpp',
    'model\Serializer.cpp',
    'model\Sorts.cpp',
    'model\TreeNode.cpp',
    'model\Validator.cpp',
    'game\GameSession.cpp',
    'game\Campaign.cpp',
    'game\Json.cpp',
    'game\Leaderboard.cpp',
    'game\LevelGenerator.cpp',
    'game\LevelLoader.cpp',
    'game\LevelManager.cpp',
    'game\SaveManager.cpp',
    'game\SandboxSession.cpp',
    'game\ScoreSystem.cpp',
    'game\SnapshotBuilder.cpp',
    'gui\Assets.cpp',
    'gui\AnimationController.cpp',
    'gui\LayoutEngine.cpp',
    'gui\Renderer.cpp',
    'gui\main_gui.cpp',
    'gui\screens\BriefingScreen.cpp',
    'gui\screens\CampaignMap.cpp',
    'gui\screens\EndlessScreen.cpp',
    'gui\screens\HeapLevel.cpp',
    'gui\screens\LeaderboardScreen.cpp',
    'gui\screens\LevelScreen.cpp',
    'gui\screens\MainMenu.cpp',
    'gui\screens\PlaceholderScreen.cpp',
    'gui\screens\ReportScreen.cpp',
    'gui\screens\ResultScreen.cpp',
    'gui\screens\SandboxScreen.cpp',
    'gui\screens\SettingsScreen.cpp',
    'gui\UIManager.cpp',
    'gui\widgets\Button.cpp',
    'gui\widgets\EventLogPanel.cpp',
    'gui\widgets\Hud.cpp',
    'gui\widgets\LevelBanner.cpp',
    'gui\widgets\NineSlice.cpp',
    'gui\widgets\NumberField.cpp',
    'gui\widgets\Toolbar.cpp',
    'gui\widgets\Tooltip.cpp'
)
$sourceArgs = ($sourceFiles | ForEach-Object { '"' + (Join-Path (Join-Path $repoRoot 'DS-Project') $_) + '"' }) -join ' '
$command = 'call "' + $vcvars + '" x64 -vcvars_ver=14.44 && cl /nologo /EHsc /std:c++17 /MD /O2 /DNDEBUG /D_CONSOLE /DWIN32 /I "' + (Join-Path $repoRoot 'include') + '" /I "' + (Join-Path $repoRoot 'DS-Project') + '" /Fo:"' + $releaseDir + '/" /Fd:"' + (Join-Path $releaseDir 'DS-Project.pdb') + '" /Fe:"' + (Join-Path $releaseDir 'DS-Project.exe') + '" ' + $sourceArgs + ' /link /SUBSYSTEM:CONSOLE /LIBPATH:"' + $libDir + '" sfml-system.lib sfml-graphics.lib sfml-window.lib sfml-audio.lib'

& $env:ComSpec /d /c $command
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Copy-Item -Path (Join-Path $runtimeDir '*.dll') -Destination $releaseDir -Force
Write-Host "Built SFML game: $releaseDir\DS-Project.exe"
