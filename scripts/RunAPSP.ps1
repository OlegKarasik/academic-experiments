[CmdletBinding()]
param(
  [string] $LaunchConfigPath = './launch.launch-config',
  [string] $RunConfigPath = './run.run-config',
  [string] $EventConfigPath = './events.event-config',
  [ValidateNotNullOrEmpty()]
  [string] $OutputDirectory = $(throw 'OutputDirectory parameter is required.'),
  [int]    $Repeat = 1,
  [switch] $MeasureEnergy,
  [switch] $RunBase,
  [switch] $RunProf
)
# Constants
#
# $SourceDirectory = 'D:\Projects\Profiling';
# $ApplicationDirectory = 'D:\Projects\GitHub\academic-experiments\src\apsp\build';

$VTuneRoot = $null
if (Test-Path -Path 'C:\Program Files (x86)\Intel\oneAPI\vtune\latest' -PathType Container -ErrorAction Stop) {
    $VTuneRoot = $(Get-Item 'C:\Program Files (x86)\Intel\oneAPI\vtune\latest').Target;
};

if (($true -eq $RunProf) -and ($null -eq $VTuneRoot)) {
    Write-Host 'Unable to run profiling without VTunes being installed. Please install oneAPI first';
    return;
};

$vtune = "$VTuneRoot\bin64\vtune";
$socwatch = "$VTuneRoot\socwatch\64\socwatch";

Write-Verbose -Message "LAUNCH CONFIG PATH : $LaunchConfigPath" -ErrorAction Stop;
Write-Verbose -Message "RUN CONFIG PATH    : $RunConfigPath" -ErrorAction Stop;
Write-Verbose -Message "EVENT CONFIG PATH  : $EventConfigPath" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT DIRECTORY   : $OutputDirectory" -ErrorAction Stop;
Write-Verbose -Message "REPEAT             : $Repeat" -ErrorAction Stop;
Write-Verbose -Message "MEASURE ENERGY     : $MeasureEnergy" -ErrorAction Stop;
Write-Verbose -Message "RUN BASE           : $RunBase" -ErrorAction Stop;
Write-Verbose -Message "RUN PROFILE        : $RunProf" -ErrorAction Stop;

Write-Host "Reading launch config..." -ErrorAction Stop;
$LaunchConfig = Get-Content -Path $LaunchConfigPath -ErrorAction Stop | ConvertFrom-Json -ErrorAction Stop;

$SourceDirectory = $LaunchConfig.SourceDirectory;
$ApplicationDirectory = $LaunchConfig.ApplicationDirectory;

Write-Verbose -Message "SOURCE DIRECTORY      : $SourceDirectory" -ErrorAction Stop;
Write-Verbose -Message "APPLICATION DIRECTORY : $ApplicationDirectory" -ErrorAction Stop;

Write-Host "Reading event config..." -ErrorAction Stop;
$EventConfig = Get-Content -Path $EventConfigPath -ErrorAction Stop;

Write-Host "Reading run configuration..." -ErrorAction Stop;
$RunConfig = Get-Content -Path $RunConfigPath -ErrorAction Stop -Raw | ConvertFrom-Json -ErrorAction Stop;

Write-Host "Generating collection groups and data patterns..." -ErrorAction Stop;
$CollectionGroups = $EventConfig | % { return $_ -split ':' | Select-Object -first 1 };
$CollectionPatterns = $CollectionGroups | % { return "$_\s+(\d+)" };

Write-Host "Configuring execution environment..."
Set-Variable `
  -Name 'INTEL_LIBITTNOTIFY64' `
  -Value 'C:/Program Files (x86)/Intel/oneAPI/vtune/2021.8.0/bin64/runtime/ittnotify_collector.dll' `
  -ErrorAction Stop;

Write-Host "Starting experiements..." -ErrorAction Stop;
$OutputHashTable = @{}
$RunConfig | ForEach-Object {
  $Params = $_;
  $Params.Versions | ForEach-Object {
    $Version = $_;

    $Params.Graphs | ForEach-Object {
      $Graph = $_;
      $Graph.Args | ForEach-Object {
        $Code = $Graph.Code;

        $ArgumentsKey = $_.Key;
        $Arguments    = $_.Value;

        $ExperimentHash = @();
        $ExperimentCode = "$Code.$Version.$ArgumentsKey";

        # Resolve arguments
        #
        $Arguments = $Arguments | % { return $_ -replace '%SOURCE_DIRECTORY%',$SourceDirectory };

        Write-Host "Experiment code: $ExperimentCode" -ErrorAction Stop;

        $ExperimentOutputDirectory = Join-Path -Path $OutputDirectory -ChildPath $ExperimentCode -ErrorAction Stop;
        if (Test-Path -Path $ExperimentOutputDirectory -PathType Container -ErrorAction Stop) {
          Remove-Item -Path $ExperimentOutputDirectory -Recurse -Force -ErrorAction Stop | Out-Null;
        };
        New-Item -Path $ExperimentOutputDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

        if ($RunBase) {
          Write-Host "Running baseline..." -ErrorAction Stop;

          for ($i = 0; $i -lt $Repeat; $i = $i + 1) {
            $ExperimentIterationDirectory = Join-Path -Path $ExperimentOutputDirectory -ChildPath "$i" -ErrorAction Stop;
            $ExperimentResultsDirectory = Join-Path -Path $ExperimentIterationDirectory -ChildPath 'base' -ErrorAction Stop;

            $ExperimentOutputFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath "$Code-$Version.out" -ErrorAction Stop;
            $ExperimentResultsFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath 'cout.txt' -ErrorAction Stop;

            New-Item -Path $ExperimentResultsDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

            Write-Verbose -Message "Running application" -ErrorAction Stop;
            Write-Verbose -Message "Executable : _application-$Version.exe" -ErrorAction Stop;
            Write-Verbose -Message "Output     : $ExperimentOutputFile" -ErrorAction Stop;
            Write-Verbose -Message "Results    : $ExperimentResultsFile" -ErrorAction Stop;
            Write-Verbose -Message "Arguments  :" -ErrorAction Stop;
            $Arguments | % {
              Write-Verbose -Message "  : $_" -ErrorAction Stop;
            };

            if ($MeasureEnergy) {
              $ExperimentEnergyResultsFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath 'energy' -ErrorAction Stop;
              & $socwatch -f power -f hw-cpu-pstate `
                -o $ExperimentEnergyResultsFile `
                --program "$ApplicationDirectory\_application-$Version.exe" `
                -o $ExperimentOutputFile `
                -O 'weightlist' $Arguments `
                2> $ExperimentResultsFile 1> $null;
            } else {
              & "$ApplicationDirectory\_application-$Version.exe" `
                -o $ExperimentOutputFile `
                -O 'weightlist' $Arguments `
                2> $ExperimentResultsFile 1> $null;
            }

            if ($LastExitCode -ne 0) {
              throw "Algorithm execution has failed with exit code: '$LastExitCode'. Please investigate.";
            }

            $ExperimentHash += Get-FileHash -Path $ExperimentOutputFile -Algorithm SHA512;
          }
          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "cout\.txt" `
            -Groups 'Exec:cout-exec' `
            -PrettyGroupMatchPatterns 'Exec' `
            -PrettyGroupReplacePatterns 'Execution Time (ms)' `
            -DataPatterns 'Exec:\s+(\d+)ms' `
            -Output "cout-combined.txt" `
            -Default "0"

          if ($MeasureEnergy) {
            & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
              -NamePattern "energy\.csv" `
              -Groups 'CPU/Package_0,\s+Power:cout-power-mW' `
              -PrettyGroupMatchPatterns 'CPU/Package_0, Power' `
              -PrettyGroupReplacePatterns 'Average Rate (mW)' `
              -DataPatterns 'CPU/Package_0,\s+Power\s+,\s+([\d\.]+)' `
              -Output "cout-combined.txt" `
              -Default "0"

            & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
              -NamePattern "energy\.csv" `
              -Groups 'CPU/Package_0,\s+Power:cout-power-mJ' `
              -PrettyGroupMatchPatterns 'CPU/Package_0, Power' `
              -PrettyGroupReplacePatterns 'Total (mJ)' `
              -DataPatterns 'CPU/Package_0,\s+Power\s+,\s+[\d\.]+\s+,\s+([\d\.]+)' `
              -Output "cout-combined.txt" `
              -Default "0"

            $p = @();

            $patterns = @();
            for ($i = 0; $i -lt [System.Environment]::ProcessorCount; $i++) {
              $patterns += , "P\d+\s+,\s+\d+\s--\s\d+\s+(?:,\s+([\d\.]+)\s+){$($i + 1)}"
            }

            $p += , $patterns;

            & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
              -NamePattern "energy\.csv" `
              -Headline "P-States" `
              -Groups 'P\d+\s+,\s+\d+\s--\s\d+:cout-hw-cpu-pstate' `
              -PrettyGroupMatchPatterns 'P(\d+)\s+,' `
              -PrettyGroupReplacePatterns 'P$1:' `
              -DataPatterns $p `
              -Output "cout-combined.txt" `
              -Default "0"
          }

          Copy-Item -Path $(Join-Path -Path $ExperimentOutputDirectory -ChildPath "cout-combined.txt" -ErrorAction Stop) `
                    -Destination $(Join-Path -Path $OutputDirectory -ChildPath "$($ExperimentCode).-cout.txt" -ErrorAction Stop) `
                    -ErrorAction Stop
        }

        if ($RunProf) {
          Write-Host "Running profiling..." -ErrorAction Stop;

          for ($i = 0; $i -lt $Repeat; $i = $i + 1) {
            $ExperimentIterationDirectory = Join-Path -Path $ExperimentOutputDirectory -ChildPath "$i" -ErrorAction Stop;
            $ExperimentResultsDirectory = Join-Path -Path $ExperimentIterationDirectory -ChildPath 'prof' -ErrorAction Stop;

            $ExperimentOutputFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath "$Code-$Version.out" -ErrorAction Stop;
            $ExperimentResultsFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath 'vtune-cout.txt' -ErrorAction Stop;

            New-Item -Path $ExperimentResultsDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

            Write-Verbose -Message "Running application (vTune)" -ErrorAction Stop;
            Write-Verbose -Message "Executable : _application-$Version-itt.exe" -ErrorAction Stop;
            Write-Verbose -Message "Output     : $ExperimentOutputFile" -ErrorAction Stop;
            Write-Verbose -Message "Results    : $ExperimentResultsFile" -ErrorAction Stop;
            Write-Verbose -Message "Arguments  :" -ErrorAction Stop;
            $Arguments | % {
              Write-Verbose -Message "  : $_" -ErrorAction Stop;
            };

            & $vtune -collect-with runsa `
              -no-summary -result-dir $ExperimentResultsDirectory `
              -knob enable-user-tasks=true `
              -knob event-config=$($EventConfig -join ',') `
              -allow-multiple-runs `
              -data-limit=10000 `
              --app-working-dir=$ApplicationDirectory `
              -- "$ApplicationDirectory\_application-$Version-itt.exe" -o $ExperimentOutputFile -O 'weightlist' $Arguments `
              2> $ExperimentResultsFile 1> $null;

            if ($LastExitCode -ne 0) {
              throw "Algorithm execution (profiled) has failed with exit code: '$LastExitCode'. Please investigate.";
            }

            $ExperimentHash += Get-FileHash -Path $ExperimentOutputFile -Algorithm SHA512;

            Write-Verbose -Message "Running analysis (vTune)" -ErrorAction Stop;

            & $vtune -R summary `
              -result-dir $ExperimentResultsDirectory `
              -filter 'task=apsp.shell.exec' `
              > $(Join-Path -Path $ExperimentResultsDirectory -ChildPath 'vtune.txt' -ErrorAction Stop);

            if ($LastExitCode -ne 0) {
              throw "Profiling report generation has failed with exit code: '$LastExitCode'. Please investigate.";
            }
          }
          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "vtune-cout\.txt" `
            -Groups 'Exec:vtune-exec' `
            -PrettyGroupMatchPatterns 'Exec' `
            -PrettyGroupReplacePatterns 'Execution Time (ms)' `
            -DataPatterns 'Exec:\s+(\d+)ms' `
            -Output "vtune-combined.txt" `
            -Multiple `
            -Default "0"

          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "vtune\.txt" `
            -Headline "Hardware Events" `
            -Groups $CollectionGroups `
            -DataPatterns $CollectionPatterns `
            -Output "vtune-combined.txt" `
            -Default "0"

            Copy-Item -Path $(Join-Path -Path $ExperimentOutputDirectory -ChildPath "vtune-combined.txt" -ErrorAction Stop) `
                      -Destination $(Join-Path -Path $OutputDirectory -ChildPath "$($ExperimentCode).-vtune.txt" -ErrorAction Stop) `
                      -ErrorAction Stop
        }

        $OutputHashTable[$Code] += $ExperimentHash;
      }
    }
  }
}

Write-Host "Starting verification..." -ErrorAction Stop;
$OutputHashTable.Keys | % {
  $OutputHash = $OutputHashTable[$_];

  Write-Host "Comparing $($OutputHash.Count) file hashes..."
  $UniqueHash = $OutputHash | Group-Object -Property 'Hash' -AsHashtable
  if ($UniqueHash.Count -eq 1) {
    Write-Host "Verification completed. No failures detected."
  } else {
    Write-Host "Verification completed. Detected multiple hash origins:"
    $UniqueHash.Keys | % {
      Write-Host $_
      $UniqueHash[$_] | % {
        Write-Host $_.Path
      }
    }
  }
}

