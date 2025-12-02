<#
  push.ps1
  Simple safe Git auto-push script for Windows PowerShell.
  Usage:  .\push.ps1 -Message "commit message"
#>

param(
  [string]$Message = ""
)

# --- go to script folder (so script works when double-clicked)
$scriptPath = Split-Path -Path $MyInvocation.MyCommand.Definition -Parent
Set-Location $scriptPath

# ensure this is a git repo
if (-not (Test-Path ".git")) {
    Write-Error "This folder is not a git repository."
    exit 1
}

# show changed files (porcelain)
$status = git status --porcelain
if ([string]::IsNullOrWhiteSpace($status)) {
    Write-Host "No changes to commit. Nothing to push."
    exit 0
}

Write-Host "Files changed:"
git status --porcelain | ForEach-Object { Write-Host $_ }

# default message if none provided
if ([string]::IsNullOrWhiteSpace($Message)) {
    $dt = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $Message = "Auto-update: $dt"
}

# warn about credentials.txt if not in .gitignore
if (Test-Path ".gitignore") {
    $gi = Get-Content .gitignore -Raw
    if ($gi -notmatch "(?m)^\s*credentials.txt\s*$") {
        Write-Warning "Make sure credentials.txt is in .gitignore to avoid committing secrets."
    }
} else {
    Write-Warning ".gitignore not found - create one to ignore credentials.txt and build artifacts."
}

# stage changes
git add -A

# commit
git commit -m $Message

if ($LASTEXITCODE -ne 0) {
    Write-Error "Commit failed (maybe nothing to commit or merge conflict). Aborting."
    exit 2
}

# push to origin main
git push origin main

if ($LASTEXITCODE -ne 0) {
    Write-Error "Push failed. You might need to authenticate or resolve conflicts."
    exit 3
}

Write-Host "Push successful."
exit 0
