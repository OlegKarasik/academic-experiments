[CmdletBinding()]
param(
  [ValidateNotNullOrEmpty()]
  [string] $ClustersConfigPath       = $(throw './clusters.clusters-config'),
  [ValidateNotNullOrEmpty()]
  [string] $ConnectionEdgePercentage = $(throw '-ConnectionEdgePercentage parameter is required'),
  [ValidateNotNullOrEmpty()]
  [string] $OutputDirectory          = $(throw '-OutputDirectory parameter is required'),
  [string] $ToolsDirectory           = '',
  [int]    $MinWeight                = 1,
  [int]    $MaxWeight                = 100
)

Write-Verbose -Message "CLUSTERS CONFIG PATH         : $ClustersConfigPath" -ErrorAction Stop;
Write-Verbose -Message "OUTPUT DIRECTORY             : $OutputDirectory" -ErrorAction Stop;
Write-Verbose -Message "CONNECTION EDGE PERCENTAGE   : $ConnectionEdgePercentage" -ErrorAction Stop;
Write-Verbose -Message "MIN WEIGHT                   : $MinWeight" -ErrorAction Stop;
Write-Verbose -Message "MAX WEIGHT                   : $MaxWeight" -ErrorAction Stop;

if (-not (Test-Path -Path $ClustersConfigPath -PathType Leaf -ErrorAction Stop)) {
  throw "File '$ClustersConfigPath' does not exist";
};
if (-not (Test-Path -Path $OutputDirectory -PathType Container -ErrorAction Stop)) {
  throw "Directory '$OutputDirectory' does not exist";
};
if ($MinWeight -eq 0) {
  throw "-MinWeight parameter can't be 0";
};

$GraphGeneratorsExecutable = $null;
$GraphConvertersExecutable = $null;
$GraphAnalysersExecutable  = $null;

if ($ToolsDirectory -eq '') {
  $Paths = @(
    './../src/tools/build/mingw-gcc-release',
    './../src/tools/build/osx-llvm-clang-release'
  );

  for ($i = 0; $i -lt $Paths.Length; ++$i) {
    $ToolsDirectory            = Join-Path -Path $PSScriptRoot -ChildPath './../src/tools/build/mingw-gcc-release' -Resolve;
    $GraphGeneratorsExecutable = Join-Path -Path $ToolsDirectory -ChildPath 'graphg.exe';
    $GraphConvertersExecutable = Join-Path -Path $ToolsDirectory -ChildPath 'graphc.exe';
    $GraphAnalysersExecutable  = Join-Path -Path $ToolsDirectory -ChildPath 'grapha.exe';

    if ((Test-Path -Path $GraphGeneratorsExecutable -PathType Leaf -ErrorAction Stop) -and
        (Test-Path -Path $GraphConvertersExecutable -PathType Leaf -ErrorAction Stop) -and
        (Test-Path -Path $GraphAnalysersExecutable  -PathType Leaf -ErrorAction Stop)) {

      break;
    };
  }
}
if (-not (Test-Path -Path $GraphGeneratorsExecutable -PathType Leaf -ErrorAction Stop)) {
  throw "Tool '$GraphGeneratorsExecutable' does not exist";
};
if (-not (Test-Path -Path $GraphConvertersExecutable -PathType Leaf -ErrorAction Stop)) {
  throw "Tool '$GraphConvertersExecutable' does not exist";
};
if (-not (Test-Path -Path $GraphAnalysersExecutable -PathType Leaf -ErrorAction Stop)) {
  throw "Tool '$GraphAnalysersExecutable' does not exist";
};

Write-Verbose -Message "TOOLS DIRECTORY              : $ToolsDirectory" -ErrorAction Stop;

$ClustersConfig = Get-Content -Path $ClustersConfigPath -ErrorAction Stop -Raw | ConvertFrom-Json -ErrorAction Stop -AsHashtable;

# Pre-prcoess the input by generating random codes
# and assigning start and end indexes
#
$ClustersCode = 0;
$ClustersIndex = 0;
$ClustersConfig | ForEach-Object {
  $_.Code       = $ClustersCode;
  $_.IndexShift = $ClustersIndex;

  $ClustersCode  += 1;
  $ClustersIndex += $_.Size;
};

# Generate cluster.* files based on the input information and
# read all edges into global edges array
#
$Index = 0;
$Edges = @();

$MeasureClustersGeneration = Measure-Command {
  $ClustersConfig | ForEach-Object {
    $C = $_;

    # Save the current length of vertices array
    # as 'zero' index of the cluster' edges (in the global array).
    #
    $C.StartIndex = $Edges.Length;

    & $GraphGeneratorsExecutable `
      -o "$($OutputDirectory)\intermediate.$($C.Code).g" `
      -O edgelist `
      -a "$($C.Type)" `
      -v "$($C.Size)" `
      2> $null 1> $null;

    $ClusterEdges = Get-Content -Path "$($OutputDirectory)\intermediate.$($C.Code).g" | ForEach-Object {
      $Success = $_ -match '(?<X>\d+)\s(?<Y>\d+)';
      if ($Success -eq $false) {
        throw 'There is a mistake in the generated graph, the format doesn''t match edgelist';
      }

      $X = [int]::Parse($Matches['X']) + $Index;
      $Y = [int]::Parse($Matches['Y']) + $Index;

      return @{ X = $X; Y = $Y; };
    }
    $Edges = $Edges + $ClusterEdges;

    $Index = $Index + $C.Size;

    # Save the current length of edges array
    # as the end index of cluster's edges (in the global array)
    #
    $C.EndIndex = $Edges.Length;
  };
};
Write-Verbose "[Clusters Generation Completed] $($MeasureClustersGeneration.TotalSeconds)";

$EdgesIntegrity = $Edges | ForEach-Object { return $_.X } | Sort-Object | Get-Unique;
if ($EdgesIntegrity.Length -ne $Index) {
  Write-Verbose "$($EdgesIntegrity.Length) -> $Index"

  throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
}
Write-Verbose "[State Verified]";

# Generate a random connected graph to represent
# connections between clusters (by generating the connected graph)
#
$MeasureConnectionGeneration = Measure-Command {
  & $GraphGeneratorsExecutable `
    -o "$($OutputDirectory)\intermediate.connections.g" `
    -O edgelist `
    -a 2 `
    -v "$($ClustersConfig.Length)" `
    -e $ConnectionEdgePercentage `
    2> $null 1> $null;

  # Read connections information and generate additional edges
  # to represents connections between clusters to add them
  # to global edges array
  #

  $ConnectionEdges = Get-Content -Path "$($OutputDirectory)\intermediate.connections.g" | ForEach-Object {
    $Success = $_ -match '(?<X>\d+)\s(?<Y>\d+)';
    if ($Success -eq $false) {
      throw 'There is a mistake in the generated connections graph, the format doesn''t match edgelist';
    }

    $X = $ClustersConfig[[int]::Parse($Matches['X'])];
    $Y = $ClustersConfig[[int]::Parse($Matches['Y'])];

    $XO = [System.Random]::Shared.NextInt64(0, $X.Size) + $X.IndexShift;
    $YI = [System.Random]::Shared.NextInt64(0, $Y.Size) + $Y.IndexShift;

    return @{ X = $XO; Y = $YI; }
  }
  $Edges = $Edges + $ConnectionEdges;
};
Write-Verbose "[Connection Generation Completed] $($MeasureConnectionGeneration.TotalSeconds)";

$EdgesIntegrity = $Edges | ForEach-Object { return $_.X } | Sort-Object | Get-Unique;
if ($EdgesIntegrity.Length -ne $Index) {
  Write-Verbose "$($EdgesIntegrity.Length) -> $Index"

  throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
}
Write-Verbose "[State Verified]";

# Randomise vertex values by performing random replacements
#
$MeasureShuffle = Measure-Command {
  $ShuffleSet = New-Object System.Collections.Generic.HashSet[int];
  $RandomIterations = [System.Random]::Shared.NextInt64($Index / 2, $Index);
  for ($i = 0; $i -lt $RandomIterations; ++$i) {
    $O = [System.Random]::Shared.NextInt64(0, $Index);
    $R = [System.Random]::Shared.NextInt64(0, $Index);

    if ($O -eq $R) {
      continue;
    };

    if (($ShuffleSet.Add($O) -eq $false) -or ($ShuffleSet.Add($R) -eq $false)) {
      continue;
    }

    $X = $ClustersConfig | Where-Object { ($_.IndexShift -le $O) -and (($_.IndexShift + $_.Size) -gt $O) } | Select-Object -First 1;
    $Y = $ClustersConfig | Where-Object { ($_.IndexShift -le $R) -and (($_.IndexShift + $_.Size) -gt $R) } | Select-Object -First 1;

    for ($j = $X.StartIndex; $j -lt $X.EndIndex; ++$j) {
      if ($Edges[$j].X -eq $O) { $Edges[$j].X = $R; } elseif ($Edges[$j].X -eq $R) { $Edges[$j].X = $O; }
      if ($Edges[$j].Y -eq $O) { $Edges[$j].Y = $R; } elseif ($Edges[$j].Y -eq $R) { $Edges[$j].Y = $O; }
    }
    for ($j = $Y.StartIndex; $j -lt $Y.EndIndex; ++$j) {
      if ($Edges[$j].X -eq $O) { $Edges[$j].X = $R; } elseif ($Edges[$j].X -eq $R) { $Edges[$j].X = $O; }
      if ($Edges[$j].Y -eq $O) { $Edges[$j].Y = $R; } elseif ($Edges[$j].Y -eq $R) { $Edges[$j].Y = $O; }
    }
    for ($j = ($Edges.Length - $ConnectionEdges.Length); $j -lt $Edges.Length; ++$j) {
      if ($Edges[$j].X -eq $O) { $Edges[$j].X = $R; } elseif ($Edges[$j].X -eq $R) { $Edges[$j].X = $O; }
      if ($Edges[$j].Y -eq $O) { $Edges[$j].Y = $R; } elseif ($Edges[$j].Y -eq $R) { $Edges[$j].Y = $O; }
    }
  }
}
Write-Verbose "[Shuffle Completed] $($MeasureShuffle.TotalSeconds)";

$EdgesIntegrity = $Edges | ForEach-Object { return $_.X } | Sort-Object | Get-Unique;
if ($EdgesIntegrity.Length -ne $Index) {
  Write-Verbose "$($EdgesIntegrity.Length) -> $Index"

  throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
}
Write-Verbose "[State Verified]";

# Collect clusters information
#
$ClustersConfig | ForEach-Object {
  # Iterate over the cluster's edges (from start to end)
  # and collect 'from' vertices into the array
  #
  $Vertices = $_.StartIndex .. ($_.EndIndex - 1)
            | ForEach-Object { return $Edges[$_].X; }
            | Sort-Object
            | Get-Unique;

  if ($Vertices.Length -ne $_.Size) {
    throw 'There is a mistake in the collecting information about cluster''s edges';
  }

  $_.Vertices = $Vertices;
};

# Print Edges
#
$EdgesOutput = $Edges | ForEach-Object {
  $W = [System.Random]::Shared.NextInt64($MinWeight, $MaxWeight);
  return "$($_.X) $($_.Y) $W"
};
Set-Content -Path "$($OutputDirectory)\output.g" -Value $EdgesOutput;

# Print Clusters
#
$ClustersOutput      = @();
$ClustersOutputIndex = 1;
$ClustersConfig | ForEach-Object {
  $ClustersOutput += "`$``$ClustersOutputIndex``"

  $InternalOuterIndex = 1;
  $InternalInnerIndex = 0;
  while ($InternalInnerIndex -lt $_.Vertices.Length) {
    $InternalInnerValues = @();
    for ($i = 0; ($i -lt 10) -and ($InternalInnerIndex -lt $_.Vertices.Length); ++$i, ++$InternalInnerIndex) {
      $InternalInnerValues += ($_.Vertices[$InternalInnerIndex] + 1);
    }
    $ClustersOutput += "[$InternalOuterIndex] $($InternalInnerValues -join ' ')"
    $InternalOuterIndex++;
  };
  $ClustersOutput += "";

  $ClustersOutputIndex++;
};
Set-Content -Path "$($OutputDirectory)\output.communities.g" -Value $ClustersOutput;

# Perform analysis
#
$MeasureAnalysis = Measure-Command {
  & $GraphAnalysersExecutable `
    -o "$($OutputDirectory)\output.analysis.g" `
    -g "$($OutputDirectory)\output.g" `
    -G weightlist `
    -c "$($OutputDirectory)\output.communities.g" `
    -C rlang `
    2> $null 1> $null;
}
Write-Verbose "[Analysis Generation Completed] $($MeasureAnalysis.TotalSeconds)";

$ClustersConfig | ForEach-Object {
  Remove-Item -Path "$($OutputDirectory)\intermediate.$($_.Code).g";
}
Remove-Item -Path "$($OutputDirectory)\intermediate.connections.g";
