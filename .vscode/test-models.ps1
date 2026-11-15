$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$projectRoot = Join-Path $repoRoot 'DS-Project'
$outputDir = Join-Path $projectRoot 'tests\x64\Release'
$outputExe = Join-Path $outputDir 'model_tests.exe'
$vsRoot = Join-Path $env:ProgramFiles 'Microsoft Visual Studio\18\Community'
$vcvars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat'

if (-not (Test-Path $vcvars))
{
    throw "Visual Studio C++ tools were not found at $vcvars"
}

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$sourceFiles = @(
    'model\AVLTree.cpp',
    'model\BinaryTree.cpp',
    'model\EventLog.cpp',
    'model\MaxHeap.cpp',
    'model\Validator.cpp',
    'tests\test_main.cpp',
    'tests\test_avl.cpp',
    'tests\test_order_stats.cpp',
    'tests\test_maxheap.cpp',
    'tests\test_heapsort.cpp',
    'tests\test_binary_tree.cpp'
)
$sourceArgs = ($sourceFiles | ForEach-Object { '"' + (Join-Path $projectRoot $_) + '"' }) -join ' '
$command = 'call "' + $vcvars + '" x64 -vcvars_ver=14.44 && cl /nologo /EHsc /std:c++17 /MD /I "' + $projectRoot + '" /Fo:"' + $outputDir + '/" /Fe:"' + $outputExe + '" ' + $sourceArgs

& $env:ComSpec /d /c $command
if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

& $outputExe
exit $LASTEXITCODE
