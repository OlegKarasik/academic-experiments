[CmdletBinding()]
param(
  [ValidateNotNullOrEmpty()]
  [string] $TargetDirectory = $(throw '-TargetDirectory parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $NamePattern = $(throw '-NamePattern parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $GroupPattern = $(throw '-GroupPattern parameter is required.'),
  [string] $PrettyGroupMatchPattern = $null,
  [string] $PrettyGroupReplacePattern = $null,
  [ValidateNotNullOrEmpty()]
  [string[]] $DataPatterns = $(throw '-DataPattern parameter is required.'),
  [ValidateNotNullOrEmpty()]
  [string] $Output = $(throw '-Output parameter is required.'),
  [switch] $Optional,
  [string] $OptionalDefaultValue = $null,
  [switch] $Multiple,
  $LineCount = -1
)

if (-not (Test-Path -Path $TargetDirectory -PathType Container -ErrorAction Stop)) {
  throw "Directory '$TargetDirectory' does not exist";
};
if ((($null -eq $PrettyGroupMatchPattern) -and ($null -ne $PrettyGroupReplacePattern)) -or
    (($null -ne $PrettyGroupMatchPattern) -and ($null -eq $PrettyGroupReplacePattern))) {
  throw "-PrettyGroupMatchPattern and -PrettyGroupReplacePattern must be specified both or none";
};

$Output = Join-Path -Path $TargetDirectory -ChildPath $Output -ErrorAction Stop;

Write-Verbose -Message "DIRECTORY                    : $TargetDirectory" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT                       : $Output" -ErrorAction Stop;
Write-Verbose -Message "NAME-PATTERN                 : $NamePattern" -ErrorAction Stop;
Write-Verbose -Message "GROUP-PATTERN                : $GroupPattern" -ErrorAction Stop;
Write-Verbose -Message "PRETTY-GROUP-MATCH-PATTERN   : $PrettyGroupMatchPattern" -ErrorAction Stop;
Write-Verbose -Message "PRETYY-GROUP-REPLACE-PATTERN : $PrettyGroupReplacePattern" -ErrorAction Stop;
Write-Verbose -Message "DATA-PATTERNS                :" -ErrorAction Stop;
$DataPatterns | % {
  Write-Verbose -Message "- $_" -ErrorAction Stop;
}
Write-Verbose -Message "LINE COUNT                   : $LineCount" -ErrorAction Stop;
Write-Verbose -Message "OPTIONAL                     : $Optional" -ErrorAction Stop;
Write-Verbose -Message "OPTIONAL DEFAULT GROUP       : $OptionalDefaultGroup" -ErrorAction Stop;
Write-Verbose -Message "OPTIONAL DEFAULT VALUE       : $OptionalDefaultValue" -ErrorAction Stop;
Write-Verbose -Message "MULTIPLE                     : $Multiple" -ErrorAction Stop;

$Result = @{};
$Content = @();
$Sort = { [regex]::Replace($_, '\d+', { $args[0].Value.PadLeft(20) }) }
try {
  Write-Host "Processing information from '$TargetDirectory' into '$Output'" -ErrorAction Stop;
  Get-ChildItem -Path $TargetDirectory -Recurse -Directory -ErrorAction Stop | % {
    Write-Verbose -Message "Scanning: '$($_.FullName)'" -ErrorAction Stop;
    Get-ChildItem -Path $_.FullName -File -ErrorAction Stop | ? { return $_.Name -match $NamePattern; } | Sort-Object $Sort | % {
      Write-Host "- Composing results from: '$($_.Name)'" -ErrorAction Stop;
      do {
        Get-Content -Path $_.FullName -TotalCount $LineCount -ErrorAction Stop | % {
          $Group = $null;
          $Value = $null;

          $gmatch = $_ -match $GroupPattern;
          if ($gmatch) {
            $Group = $matches[1];

            for ($i = 0; $i -lt $DataPatterns.Length; $i++) {
              $DataPattern = $DataPatterns[$i];

              # We rewrite the "group key" with an internal indexing
              # to make sure we can distinguish results from multiple
              # "data patterns".

              $GroupKey = $Group;
              if ($DataPatterns.Length -ne 1) {
                $GroupKey = "$Group ($i)"
              }
              if (($null -ne $PrettyGroupMatchPattern) -and ($null -ne $PrettyGroupReplacePattern)) {
                $GroupKey = $GroupKey -replace $PrettyGroupMatchPattern,$PrettyGroupReplacePattern
              }

              $vmatch = $_ -match $DataPattern;
              if ($vmatch) {
                $Value = $matches[1];

                if ($null -eq $Result[$GroupKey]) {
                  $Result[$GroupKey] = @($Value);
                } else {
                  $Result[$GroupKey] += , $Value;
                }
              } else {
                if ($null -ne $OptionalDefaultValue) {
                  # We can specify a default value for optional parameter (ex. 0)
                  # to have an understanding in what concrete run we had no output
                  $Result[$GroupKey] = $OptionalDefaultValue;
                }
              }
            }
          }
        };
        break;
      } while ($true);
    };
  };
  $Result.Keys | Sort-Object { [regex]::Replace($_, '\d+', { $args[0].Value.PadLeft(20) }) } -Stable | % {
    $Content += , @($_, $Result[$_]);
  }

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
