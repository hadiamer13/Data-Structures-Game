$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$projectRoot = Join-Path $repoRoot 'DS-Project'
$outputDir = Join-Path $projectRoot 'benchmarks\x64\Release'
$outputExe = Join-Path $outputDir 'model_benchmarks.exe'
$vsRoot = Join-Path $env:ProgramFiles 'Microsoft Visual Studio\18\Community'
$vcvars = Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat'

if (-not (Test-Path $vcvars)) { throw "Visual Studio C++ tools were not found at $vcvars" }
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$sourceFiles = @(
    'model\AVLTree.cpp', 'model\BinaryTree.cpp', 'model\BST.cpp',
    'model\EventLog.cpp', 'model\MaxHeap.cpp', 'model\Serializer.cpp',
    'model\Sorts.cpp', 'model\TreeNode.cpp', 'model\Validator.cpp',
    'benchmarks\bench_main.cpp', 'benchmarks\bench_height_search.cpp',
    'benchmarks\bench_rotations_kth.cpp', 'benchmarks\bench_heap.cpp',
    'benchmarks\bench_sorting.cpp'
)
$sourceArgs = ($sourceFiles | ForEach-Object { '"' + (Join-Path $projectRoot $_) + '"' }) -join ' '
$command = 'call "' + $vcvars + '" x64 -vcvars_ver=14.44 && cl /nologo /EHsc /std:c++17 /MD /O2 /DNDEBUG /I "' + $projectRoot + '" /Fo:"' + $outputDir + '/" /Fe:"' + $outputExe + '" ' + $sourceArgs
& $env:ComSpec /d /c $command
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $outputExe @args
exit $LASTEXITCODE
