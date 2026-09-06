[CmdletBinding(SupportsShouldProcess)]
param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path,
    [string]$ClangFormat = "clang-format"
)

$clangFormatCommand = Get-Command $ClangFormat -ErrorAction SilentlyContinue
if ($null -eq $clangFormatCommand) {
    throw "Cannot find '$ClangFormat'. Install LLVM or pass -ClangFormat with its full path."
}

$rootPath = (Resolve-Path -LiteralPath $Root -ErrorAction Stop).Path
$sourceExtensions = ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"
$excludedDirectories = "\.git\\", "\\build\\", "\\Build\\", "\\CMakeFiles\\", "\\vcpkg_installed\\"

$sourceFiles = Get-ChildItem -LiteralPath $rootPath -Recurse -File |
    Where-Object {
        $_.Extension.ToLowerInvariant() -in $sourceExtensions -and
        -not ($_.FullName -match ($excludedDirectories -join "|"))
    }

if ($sourceFiles.Count -eq 0) {
    Write-Warning "No C or C++ source files found under '$rootPath'."
    return
}

foreach ($sourceFile in $sourceFiles) {
    if ($PSCmdlet.ShouldProcess($sourceFile.FullName, "Format with clang-format")) {
        & $clangFormatCommand.Source -i --style=file $sourceFile.FullName
        if ($LASTEXITCODE -ne 0) {
            throw "clang-format failed for '$($sourceFile.FullName)' with exit code $LASTEXITCODE."
        }
    }
}

$action = if ($WhatIfPreference) { "Would format" } else { "Formatted" }
Write-Host "$action $($sourceFiles.Count) C/C++ file(s) under '$rootPath'."