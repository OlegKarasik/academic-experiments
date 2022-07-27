[CmdletBinding()]
param(
  [string] $LaunchConfigPath = './launch.launch-config',
  [string] $RunConfigPath = './run.run-config',
  [string] $EventConfigPath = './events.event-config',
  [ValidateNotNullOrEmpty()]
  [string] $OutputDirectory = $(throw 'OutputDirectory parameter is required.'),
  [int]    $Repeat = 1,
  [switch] $RunBase,
  [switch] $RunProf
)
# Constants
#
# $SourceDirectory = 'D:\Projects\Profiling';
# $ApplicationDirectory = 'D:\Projects\GitHub\academic-experiments\src\apsp\shell\bin';
$vtune = 'C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune';

Write-Verbose -Message "LAUNCH CONFIG PATH : $LaunchConfigPath" -ErrorAction Stop;
Write-Verbose -Message "RUN CONFIG PATH    : $RunConfigPath" -ErrorAction Stop;
Write-Verbose -Message "EVENT CONFIG PATH  : $EventConfigPath" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT DIRECTORY   : $OutputDirectory" -ErrorAction Stop;
Write-Verbose -Message "REPEAT             : $Repeat" -ErrorAction Stop;
Write-Verbose -Message "RUN BASE           : $RunBase" -ErrorAction Stop;
Write-Verbose -Message "RUN PROFILE        : $RunProf" -ErrorAction Stop;

Write-Host "Reading launch config..." -ErrorAction Stop;
$LaunchConfig = Get-Content -Path $LaunchConfigPath -ErrorAction Stop | ConvertFrom-Json -ErrorAction Stop;

$SourceDirectory = $LaunchConfig.source_dir;
$ApplicationDirectory = $LaunchConfig.app_dir;

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
$OutputHash = @()
$RunConfig | ForEach-Object {
  $Params = $_;
  $Params.versions | ForEach-Object {
    $Version = $_;

    $Params.graphs | ForEach-Object {
      $Graph = $_;
      $Graph.args | ForEach-Object {
        $Size = $Graph.size;
        $Arguments = $_;
        $ExperimentCode = "$Size.$Version.$($Arguments -join '')";

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

            $ExperimentInputFile = Join-Path -Path $SourceDirectory -ChildPath "$Size.g" -ErrorAction Stop;
            $ExperimentOutputFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath "$Size.g-$version.out" -ErrorAction Stop;
            $ExperimentResultsFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath 'cout.txt' -ErrorAction Stop;

            New-Item -Path $ExperimentResultsDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

            Write-Verbose -Message "Running application" -ErrorAction Stop;
            Write-Verbose -Message "Executable : _application-$version.exe" -ErrorAction Stop;
            Write-Verbose -Message "Input      : $ExperimentInputFile" -ErrorAction Stop;
            Write-Verbose -Message "Output     : $ExperimentOutputFile" -ErrorAction Stop;
            Write-Verbose -Message "Results    : $ExperimentResultsFile" -ErrorAction Stop;

            & "$ApplicationDirectory\_application-$version.exe" `
              -i $ExperimentInputFile `
              -o $ExperimentOutputFile $arguments `
              2> $ExperimentResultsFile;

            if ($LastExitCode -ne 0) {
              throw "Algorithm execution has failed with exit code: '$LastExitCode'. Please investigate.";
            }

            $OutputHash += Get-FileHash -Path ExperimentOutputFile -Algorithm SHA512;
          }
          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "cout\.txt" `
            -Groups 'Exec:cout-exec' `
            -DataPatterns 'Exec:\s+(\d+)ms' `
            -Output "cout-combined.txt" `
            -Default "0"

          Copy-Item -Path $(Join-Path -Path $ExperimentOutputDirectory -ChildPath "cout-combined.txt" -ErrorAction Stop) `
                    -Destination $(Join-Path -Path $OutputDirectory -ChildPath "$($ExperimentCode).-cout.txt" -ErrorAction Stop) `
                    -ErrorAction Stop
        }

        if ($RunProf) {
          Write-Host "Running profiling..." -ErrorAction Stop;

          for ($i = 0; $i -lt $Repeat; $i = $i + 1) {
            $ExperimentIterationDirectory = Join-Path -Path $ExperimentOutputDirectory -ChildPath "$i" -ErrorAction Stop;
            $ExperimentResultsDirectory = Join-Path -Path $ExperimentIterationDirectory -ChildPath 'prof' -ErrorAction Stop;

            $ExperimentInputFile = Join-Path -Path $SourceDirectory -ChildPath "$Size.g" -ErrorAction Stop;
            $ExperimentOutputFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath "$Size.g-$version.out" -ErrorAction Stop;
            $ExperimentResultsFile = Join-Path -Path $ExperimentResultsDirectory -ChildPath 'cout.txt' -ErrorAction Stop;

            New-Item -Path $ExperimentResultsDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

            Write-Verbose -Message "Running application (vTune)" -ErrorAction Stop;
            Write-Verbose -Message "Executable : _application-$version-itt.exe" -ErrorAction Stop;
            Write-Verbose -Message "Input      : $ExperimentInputFile" -ErrorAction Stop;
            Write-Verbose -Message "Output     : $ExperimentOutputFile" -ErrorAction Stop;
            Write-Verbose -Message "Results    : $ExperimentResultsFile" -ErrorAction Stop;

            & $vtune -collect-with runsa `
              -no-summary -result-dir $ExperimentResultsDirectory `
              -knob enable-user-tasks=true `
              -knob event-config=$($EventConfig -join ',') `
              -allow-multiple-runs `
              -data-limit=10000 `
              --app-working-dir=$ApplicationDirectory `
              -- "$ApplicationDirectory\_application-$version-itt.exe" -i $ExperimentInputFile -o $ExperimentOutputFile $arguments `
              2> $ExperimentResultsFile;

            if ($LastExitCode -ne 0) {
              throw "Algorithm execution (profiled) has failed with exit code: '$LastExitCode'. Please investigate.";
            }

            $OutputHash += Get-FileHash -Path ExperimentOutputFile -Algorithm SHA512;

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
            -DataPatterns 'Exec:\s+(\d+)ms' `
            -Output "vtune-cout-combined.txt" `
            -Multiple `
            -Default "0"

            Copy-Item -Path $(Join-Path -Path $ExperimentOutputDirectory -ChildPath "vtune-cout-combined.txt" -ErrorAction Stop) `
                      -Destination $(Join-Path -Path $OutputDirectory -ChildPath "$($ExperimentCode).-vtune-cout.txt" -ErrorAction Stop) `
                      -ErrorAction Stop

          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "vtune\.txt" `
            -Groups $CollectionGroups `
            -DataPatterns $CollectionPatterns `
            -Output "vtune-combined.txt" `
            -Default "0"

            Copy-Item -Path $(Join-Path -Path $ExperimentOutputDirectory -ChildPath "vtune-combined.txt" -ErrorAction Stop) `
                      -Destination $(Join-Path -Path $OutputDirectory -ChildPath "$($ExperimentCode).-vtune.txt" -ErrorAction Stop) `
                      -ErrorAction Stop
        }
      }
    }
  }
}

Write-Host "Starting verification..." -ErrorAction Stop;
$UniqueHash = $OutputHash | Select-Object -Unique
if ($UniqueHash.Count -eq 1) {
  Write-Host "Verification completed. No failures detected."
} else {
  Write-Host "Verification failed for the following files:"
  $UniqueHash | % {
    Write-Host "- $($_.Path)" -ErrorAction Stop;
  }
}

