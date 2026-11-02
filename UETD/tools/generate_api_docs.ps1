$ErrorActionPreference = "Stop"

$doxygenCommand = Get-Command doxygen -CommandType Application -ErrorAction SilentlyContinue
if ($null -eq $doxygenCommand) {
    throw "Doxygen 1.9 or newer is required. Install the Doxygen CLI and add it to PATH, then run this script again."
}

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$reportDirectory = Join-Path $repositoryRoot "build\UETD\doxygen"
$doxyfile = Join-Path $repositoryRoot "UETD\Doxyfile"
$null = New-Item -ItemType Directory -Force -Path $reportDirectory

Push-Location $repositoryRoot
try {
    & $doxygenCommand.Source $doxyfile
    if ($LASTEXITCODE -ne 0) {
        throw "Doxygen failed with exit code $LASTEXITCODE. See the output above."
    }
}
finally {
    Pop-Location
}

Write-Host "HTML API reference: $reportDirectory\html\index.html"
Write-Host "XML API reference:  $reportDirectory\xml"
Write-Host "Doxygen warnings:   $reportDirectory\warnings.log"
