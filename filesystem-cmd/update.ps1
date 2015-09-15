#Requires -Version 4

# Updates MSYS2 'filesystem-cmd' package sources and PKGBUILD md5sums
# from installed package files.
#
# Reads root dir of MSYS2 installation from %MSYS2_ROOT%
#
# Author: Stefan Zimmermann <zimmermann.code@gmail.com>

$ErrorActionPreference = 'Stop'

$sourceDir = Split-Path -parent $MyInvocation.MyCommand.Source

if (-not $env:MSYS2_ROOT) {
    Write-Error "No %MSYS2_ROOT% defined."
}
# the MSYS2 cmd\ subdir, where filesystem-cmd sources get installed
$installDir = Join-Path $env:MSYS2_ROOT cmd

[Collections.ArrayList]$md5sums = @()

# state variables for PKGBUILD parsing
$inSourceBlock = $false
$inMD5Block = $false
# parse PKGBUILD content, overwrite sources from installed files,
# compute new MD5 hashes, and rewrite PKGBUILD
$updatedContent = foreach ($line in Get-Content PKGBUILD) {
    if ($line -match '^source=\(') {
        $inSourceBlock = $true
    }
    if ($inSourceBlock) {
        if ($line -match "'([^']+)'") {
            $source = $matches[1]
            # if source name has 2 '.' then 1. part is subdir for installation
            if (($source -split '\.').Length -eq 3) {
                $dir, $file = $source -split '\.', 2
                $installedFile = Join-Path $dir $file
            } else {
                $installedFile = $source
            }
            $source = Join-Path $sourceDir $source
            Copy-Item (Join-Path $installDir $installedFile) $source
            $md5 = Get-FileHash $source -Algorithm MD5
            [void]$md5sums.Add($md5.Hash)
        }
        if ($line -match '\)') {
            $inSourceBlock = $false
        }
    }
    if ($line -match '^md5sums=\(') {
        $inMD5Block = $true
    }
    if ($inMD5Block) {
        if ($line -match "'([^']+)'") {
            $line = $line -replace $matches[1], $md5sums[0]
            $md5sums.RemoveAt(0)
        }
        if ($line -match '\)') {
            $inMD5Block = $false
        }
    }
    $line.TrimEnd()
}
# write PKGBUILD with unix line endings
[IO.File]::WriteAllText((Join-Path $sourceDir PKGBUILD),
                        $updatedContent -join "`n")
