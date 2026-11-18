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
    'game\Json.cpp',
    'game\Leaderboard.cpp',
    'game\LevelGenerator.cpp',
    'game\LevelLoader.cpp',
    'game\SaveManager.cpp',
    'game\ScoreSystem.cpp',
    'game\SnapshotBuilder.cpp',
    'gui\Assets.cpp',
    'gui\main_gui.cpp',
    'gui\screens\BriefingScreen.cpp',
    'gui\screens\CampaignMap.cpp',
    'gui\screens\MainMenu.cpp',
    'gui\screens\PlaceholderScreen.cpp',
    'gui\UIManager.cpp',
    'gui\widgets\Button.cpp',
    'gui\widgets\LevelBanner.cpp',
    'gui\widgets\LevelNode.cpp',
    'tests\test_main.cpp',
    'tests\test_avl.cpp',
    'tests\test_bst.cpp',
    'tests\test_binary_tree.cpp',
    'tests\test_campaign_support.cpp',
    'tests\test_containers.cpp',
    'tests\test_game_logic.cpp',
    'tests\test_heapsort.cpp',
    'tests\test_maxheap.cpp',
    'tests\test_node_pool.cpp',
    'tests\test_order_stats.cpp',
    'tests\test_serializer.cpp',
    'tests\test_sorts.cpp',
    'tests\test_validator.cpp'
)
$sourceArgs = ($sourceFiles | ForEach-Object { '"' + (Join-Path (Join-Path $repoRoot 'DS-Project') $_) + '"' }) -join ' '
$command = 'call "' + $vcvars + '" x64 -vcvars_ver=14.44 && cl /nologo /EHsc /std:c++17 /MD /O2 /DNDEBUG /D_CONSOLE /DWIN32 /I "' + (Join-Path $repoRoot 'include') + '" /I "' + (Join-Path $repoRoot 'DS-Project') + '" /Fo:"' + $releaseDir + '/" /Fd:"' + (Join-Path $releaseDir 'DS-Project.pdb') + '" /Fe:"' + (Join-Path $releaseDir 'DS-Project.exe') + '" ' + $sourceArgs + ' /link /SUBSYSTEM:CONSOLE /LIBPATH:"' + $libDir + '" sfml-system.lib sfml-graphics.lib sfml-window.lib sfml-audio.lib'

& $env:ComSpec /d /c $command
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Copy-Item -Path (Join-Path $runtimeDir '*.dll') -Destination $releaseDir -Force
Write-Host "Built full project with doctest entry point: $releaseDir\DS-Project.exe"
