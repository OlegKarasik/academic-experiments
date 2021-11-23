[CmdletBinding()]
param(
  [ValidateNotNullOrEmpty()]
  [string]   $TargetDirectory = $(throw '-TargetDirectory parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string]   $NamePattern = $(throw '-NamePattern parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string[]] $Groups = $(throw '-Groups parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string[]] $DataPatterns = $(throw '-DataPatterns parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string]   $Output = $(throw '-Output parameter is required.'),
  [string]   $Default = $null,
  [switch]   $Multiple,
  $LineCount = -1
)

if ($Groups.Count -ne $DataPatterns.Count) {
  throw 'Number of -Groups and -DataPatterns should match';
}

$Output = Join-Path -Path $TargetDirectory -ChildPath $Output -ErrorAction Stop;
$Aliases = @($null) * $Groups.Length;
for ($i = 0; $i -lt $Groups.Count; $i = $i + 1) {
  $Split = $Groups[$i] -split ':';
  if ($Split.Length -eq 1) {
    $Split += $Groups[$i];
  }

  $Groups[$i] = $Split[0];
  $Aliases[$i] = $Split[1];
}

Write-Verbose -Message "DIRECTORY     : $TargetDirectory" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT        : $Output" -ErrorAction Stop;
Write-Verbose -Message "NAME-PATTERN  : $NamePattern" -ErrorAction Stop;
Write-Verbose -Message "GROUPS        :";
$Groups | % {
  Write-Verbose -Message "- $_" -ErrorAction Stop;
}
Write-Verbose -Message "ALIASES       :";
$Aliases | % {
  Write-Verbose -Message "- $_" -ErrorAction Stop;
}
Write-Verbose -Message "DATA-PATTERNS : $DataPattern" -ErrorAction Stop;
$DataPatterns | % {
  Write-Verbose -Message "- $_" -ErrorAction Stop;
}
Write-Verbose -Message "DEFAULT       : $Default" -ErrorAction Stop;
Write-Verbose -Message "MULTIPLE      : $Multiple" -ErrorAction Stop;
Write-Verbose -Message "LINE COUNT    : $LineCount" -ErrorAction Stop;

for ($i = 0; $i -lt $Groups.Count; $i = $i + 1) {
  & "$PSScriptRoot/ComposeResults.ps1" -TargetDirectory $TargetDirectory `
    -NamePattern $NamePattern `
    -GroupPattern "($($Groups[$i]))" `
    -DataPattern $DataPatterns[$i] `
    -Output "$($($Aliases[$i])).txt" `
    -LineCount $LineCount `
    -ErrorAction Stop `
    -Optional `
    -OptionalDefault $Default `
    -Multiple:$Multiple
}

Write-Host "Processing outputs from '$TargetDirectory' into '$Output'" -ErrorAction Stop;
$Aliases | % {
  $Target = Join-Path -Path $TargetDirectory -ChildPath "$_.txt" -ErrorAction Stop;

  Write-Verbose -Message "Probing: '$Target'" -ErrorAction Stop;
  if (-not (Test-Path -Path $Target -PathType Leaf -ErrorAction Stop)) {
    Write-Verbose -Message "[Missing]" -ErrorAction Stop;
    return;
  };
  Write-Verbose -Message "[Found]" -ErrorAction Stop;

  Write-Host "- Combining output from '$Target'" -ErrorAction Stop;

  $Content = Get-Content -Path $Target -ErrorAction Stop;

  Add-Content -Path $Output -Value $Content -ErrorAction Stop;
}
Write-Host "[Done]" -ForegroundColor Green -ErrorAction Stop;
