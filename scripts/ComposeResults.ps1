[CmdletBinding()]
param(
  [ValidateNotNullOrEmpty()]
  [string] $TargetDirectory = $(throw '-TargetDirectory parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $NamePattern = $(throw '-NamePattern parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $GroupPattern = $(throw '-GroupPattern parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $DataPattern = $(throw '-DataPattern parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $Output = $(throw '-Output parameter is required.'),
  [switch] $Optional,
  [string] $OptionalDefault = $null,
  [switch] $Multiple,
  $LineCount = -1
)

if (-not (Test-Path -Path $TargetDirectory -PathType Container -ErrorAction Stop)) {
  throw "Directory '$TargetDirectory' does not exist";
};

$Output = Join-Path -Path $TargetDirectory -ChildPath $Output -ErrorAction Stop;

Write-Verbose -Message "DIRECTORY        : $TargetDirectory" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT           : $Output" -ErrorAction Stop;
Write-Verbose -Message "NAME-PATTERN     : $NamePattern" -ErrorAction Stop;
Write-Verbose -Message "GROUP-PATTERN    : $GroupPattern" -ErrorAction Stop;
Write-Verbose -Message "DATA-PATTERN     : $DataPattern" -ErrorAction Stop;
Write-Verbose -Message "LINE COUNT       : $LineCount" -ErrorAction Stop;
Write-Verbose -Message "OPTIONAL         : $Optional" -ErrorAction Stop;
Write-Verbose -Message "OPTIONAL DEFAULT : $OptionalDefault" -ErrorAction Stop;
Write-Verbose -Message "MULTIPLE         : $Multiple" -ErrorAction Stop;

$Content = @();
$Sort = { [regex]::Replace($_, '\d+', { $args[0].Value.PadLeft(20) }) }
try {
  Write-Host "Processing information from '$TargetDirectory' into '$Output'" -ErrorAction Stop;
  Get-ChildItem -Path $TargetDirectory -Recurse -Directory -ErrorAction Stop | % {
    Write-Verbose -Message "Scanning: '$($_.FullName)'" -ErrorAction Stop;
    Get-ChildItem -Path $_.FullName -File -ErrorAction Stop | ? { return $_.Name -match $NamePattern; } | Sort-Object $Sort | % {
      Write-Host "- Composing results from: '$($_.Name)'" -ErrorAction Stop;
      $Values = @();
      $Value = $null;
      $Group = $null;
      do {
        Get-Content -Path $_.FullName -TotalCount $LineCount -ErrorAction Stop | % {
          if ($Group -eq $null) {
            $match = $_ -match $GroupPattern;
            if ($match) {
              $match[0] -match $GroupPattern | Out-Null;
              $Group = $matches[1];
            };
          };
          if ($Value -eq $null) {
            $match = $_ -match $DataPattern;
            if ($match) {
              $match[0] -match $DataPattern | Out-Null;
              $Value = $matches[1];
            };
          };
          if (($Group -ne $null) -and ($Value -ne $null)) {
            $Values += $Value;
            if ($Multiple -eq $true) {
              # If we are configured to hold multiple matches then we need
              # to make sure we aren't break and continue scan
              $Value = $null;
            } else {
              break;
            }
          };
        };
        break;
      } while ($true);
      if (($Group -eq $null) -or ($Values.Length -eq 0)) {
        if ($Optional -eq $false) {
          throw 'Pattern search failed with the following error: -GroupPattern or -DataPattern has no result';
        }
        if ($OptionalDefault -ne $null) {
          # We can specify a default value for optional parameter (ex. 0)
          # to have an understanding in what concrete run we had no output
          $Values += $OptionalDefault;
        } else {
          return;
        }
      };

      $Found = $false;
      for ($i = 0; $i -lt $Content.Length; ++$i) {
        if ($Content[$i][0] -eq $Group) {
          $Found = $true;
          $Content[$i][1] += $Values;

          return;
        };
      };
      if (-not $Found) {
        $Content += , @($Group, $Values);
      };
    };
  };
  $ContentLine = @();
  for ($i = 0; $i -lt $Content.Length; ++$i) {
    $ContentLine += "$($Content[$i][0]) $($($Content[$i][1]) -join ' ')";
  };
  $ContentLine = $ContentLine -join $([System.Environment]::NewLine);
  if ($ContentLine.Length -ne 0) {
    Set-Content -Path $Output -Value $ContentLine -ErrorAction Stop;
  };
}
catch {
  throw $_;
};
Write-Host "[Done]" -ForegroundColor Green -ErrorAction Stop;
