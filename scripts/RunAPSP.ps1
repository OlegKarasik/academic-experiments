[CmdletBinding()]
param(
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
$source_dir = 'D:\Projects\Profiling';
$app_dir = 'D:\Projects\GitHub\academic-experiments\src\apsp\shell\bin';

$vtune = 'C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune';

Write-Verbose -Message "RUN CONFIG PATH   : $RunConfigPath" -ErrorAction Stop;
Write-Verbose -Message "EVENT CONFIG PATH : $EventConfigPath" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT DIRECTORY  : $OutputDirectory" -ErrorAction Stop;
Write-Verbose -Message "REPEAT            : $Repeat" -ErrorAction Stop;
Write-Verbose -Message "RUN BASE          : $RunBase" -ErrorAction Stop;
Write-Verbose -Message "RUN PROFILE       : $RunProf" -ErrorAction Stop;

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
  -Value 'C:/Program Files (x86)/Intel/oneAPI/vtune/2021.8.0/bin64\runtime\ittnotify_collector.dll' `
  -ErrorAction Stop;

Write-Host "Starting experiements..." -ErrorAction Stop;
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
            
            New-Item -Path $ExperimentResultsDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

            & "$app_dir\_application-$version.exe" `
              -i "$source_dir\$Size.g" `
              -o "$source_dir\$Size.g.out" $arguments `
              2> $(Join-Path -Path $ExperimentResultsDirectory -ChildPath 'cout.txt' -ErrorAction Stop);
          }
          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "cout\.txt" `
            -Groups 'Exec' `
            -DataPatterns 'Exec:\s+(\d+)ms' `
            -Output "cout-combined.txt"
        }

        if ($RunProf) {
          Write-Host "Running profiling..." -ErrorAction Stop;

          for ($i = 0; $i -lt $Repeat; $i = $i + 1) {
            $ExperimentIterationDirectory = Join-Path -Path $ExperimentOutputDirectory -ChildPath "$i" -ErrorAction Stop;
            $ExperimentResultsDirectory = Join-Path -Path $ExperimentIterationDirectory -ChildPath 'prof' -ErrorAction Stop;
            
            New-Item -Path $ExperimentResultsDirectory -ItemType Directory -ErrorAction Stop | Out-Null;

            & $vtune -collect-with runsa `
              -no-summary -result-dir $ExperimentResultsDirectory `
              -knob enable-user-tasks=true `
              -knob event-config=$($EventConfig -join ',') `
              -allow-multiple-runs `
              --app-working-dir=$app_dir `
              -- $app_dir\_application-$version.exe -i "$source_dir\$Size.g" -o "$source_dir\$Size.g.out" $arguments `
              2> $(Join-Path -Path $ExperimentResultsDirectory -ChildPath 'vtune-cout.txt' -ErrorAction Stop)

            & $vtune -R summary `
              -result-dir $ExperimentResultsDirectory `
              -filter 'task=apsp.shell.exec' `
              > $(Join-Path -Path $ExperimentResultsDirectory -ChildPath 'vtune.txt' -ErrorAction Stop);
          }
          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "vtune-cout\.txt" `
            -Groups 'Exec' `
            -DataPatterns 'Exec:\s+(\d+)ms' `
            -Output "vtune-cout-combined.txt" `
            -Multiple

          & "$PSScriptRoot/ComposeGroupsResults.ps1" -TargetDirectory $ExperimentOutputDirectory `
            -NamePattern "vtune\.txt" `
            -Groups $CollectionGroups `
            -DataPatterns $CollectionPatterns `
            -Output "vtune-combined.txt"
        }
      }
    }
  }
}