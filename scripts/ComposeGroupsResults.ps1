[CmdletBinding()]
param(
    [ValidateNotNullOrEmpty()]
    [string]$TargetDirectory = $(throw '-TargetDirectory parameter is required.'),
    [ValidateNotNullOrEmpty()]
    [string]$NamePattern = $(throw '-NamePattern parameter is required.'),
    [ValidateNotNullOrEmpty()]
    [string[]]$Groups = $(throw '-Groups parameter is required.'),
    [ValidateNotNullOrEmpty()]
    [string[]]$DataPatterns = $(throw '-DataPatterns parameter is required.'),
    [ValidateNotNullOrEmpty()]
    [string]$Output = $(throw '-Output parameter is required.'),
    $LineCount = 500
)

if ($Groups.Count -ne $DataPatterns.Count) {
    throw 'Number of -Groups and -DataPatterns should match';
}

$Output = Join-Path -Path $TargetDirectory -ChildPath $Output -ErrorAction Stop;

Write-Verbose -Message "DIRECTORY     : $TargetDirectory" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT        : $Output" -ErrorAction Stop;
Write-Verbose -Message "NAME-PATTERN  : $NamePattern" -ErrorAction Stop;
Write-Verbose -Message "GROUPS        :";
$Groups | % {
    Write-Verbose -Message "- $_" -ErrorAction Stop;
}
Write-Verbose -Message "DATA-PATTERNS : $DataPattern" -ErrorAction Stop;
$DataPatterns | % {
    Write-Verbose -Message "- $_" -ErrorAction Stop;
}
Write-Verbose -Message "LINE COUNT    : $LineCount" -ErrorAction Stop;

for ($i = 0; $i -lt $Groups.Count; $i = $i + 1) {
    & ./ComposeResults.ps1 -TargetDirectory $TargetDirectory `
                           -NamePattern $NamePattern `
                           -GroupPattern "($($Groups[$i]))" `
                           -DataPattern $DataPatterns[$i] `
                           -Output "$($Groups[$i]).out" `
                           -LineCount $LineCount `
                           -ErrorAction Stop `
                           -Optional
}

$Groups | % {
    $Target = Join-Path -Path $TargetDirectory -ChildPath "$($_).out" -ErrorAction Stop;
    if (-not (Test-Path -Path $Target -PathType Leaf -ErrorAction Stop)) {
      Write-Verbose -Message "No output for '$_', skipping";
      return;
    };

    Write-Host "Combining information from '$Target'" -ErrorAction Stop;

    $Content = Get-Content -Path $Target `
                           -ErrorAction Stop

    Add-Content -Path $Output `
                -Value $Content `
                -ErrorAction Stop
}
Write-Host "[Done]" -ForegroundColor Green -ErrorAction Stop;
