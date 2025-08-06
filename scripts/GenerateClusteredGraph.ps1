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
  New-Item -Path $OutputDirectory -ItemType Directory -ErrorAction Stop | Out-Null;
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

$Index      = 0;
$EdgesCount = 0;

$VertexConnectionCounts = [int[]]::new($ClustersIndex);
$VertexConnections      = [System.Collections.Generic.Dictionary[int,System.Collections.Generic.List[int]]]::new($ClustersIndex);

$MeasureClustersGeneration = Measure-Command {

  # Generate cluster.* files based on the input information
  #
  $ClustersConfig | ForEach-Object {
    $C = $_;

    & $GraphGeneratorsExecutable `
      -o "$($OutputDirectory)\intermediate.$($C.Code).g" `
      -O edgelist `
      -a "$($C.Type)" `
      -v "$($C.Size)" `
      2> $null 1> $null;

    $LinesCount = 0;
    switch -File "$($OutputDirectory)\intermediate.$($C.Code).g" { default { ++$LinesCount } }

    # Set start boundry
    #
    $C.StartIndex = $EdgesCount;

    $EdgesCount = $EdgesCount + $LinesCount;

    # Set end boundry
    #
    $C.EndIndex = $EdgesCount;
  }

  # Generate a random connected graph to represent
  # connections between clusters (by generating the connected graph)
  #
  & $GraphGeneratorsExecutable `
    -o "$($OutputDirectory)\intermediate.connections.g" `
    -O edgelist `
    -a 2 `
    -v "$($ClustersConfig.Length)" `
    -e $ConnectionEdgePercentage `
    2> $null 1> $null;

    $LinesCount = 0;
    switch -File "$($OutputDirectory)\intermediate.connections.g" { default { ++$LinesCount } }

    $EdgesCount = $EdgesCount + $LinesCount;
}
Write-Verbose "[Clusters Generation Completed] $($MeasureClustersGeneration.TotalSeconds)";

$EdgesX     = [int[]]::new($EdgesCount);
$EdgesY     = [int[]]::new($EdgesCount);
$EdgesPrint = [string[]]::new($EdgesCount);

$EdgesLineIndex  = 0;

$MeasureClustersDeserialisation = Measure-Command {
  $ClustersConfig | ForEach-Object {
    $C = $_;
    $G = "$($OutputDirectory)\intermediate.$($C.Code).g";

    $Reader = New-Object -TypeName System.IO.StreamReader -ArgumentList $G
    while (!$Reader.EndOfStream) {
        $Items = $Reader.ReadLine().Split();

        $X = [int]::Parse($Items[0]) + $Index;
        $Y = [int]::Parse($Items[1]) + $Index;

        $EdgesX[$EdgesLineIndex] = $X;
        $EdgesY[$EdgesLineIndex] = $Y;

        $EdgesLineIndex = $EdgesLineIndex + 1;

        $VertexConnectionCounts[$X] = $VertexConnectionCounts[$X] + 1;
        $VertexConnectionCounts[$Y] = $VertexConnectionCounts[$Y] + 1;
    }
    $Reader.Close()

    $Index = $Index + $C.Size;
  };
};
Write-Verbose "[Clusters Deserialisation Completed] $($MeasureClustersDeserialisation.TotalSeconds)";

$MeasureStateVerification = Measure-Command {
  $EdgesIntegrityX = [System.Collections.Generic.HashSet[int]]::new($EdgesX)
  $EdgesIntegrityY = [System.Collections.Generic.HashSet[int]]::new($EdgesY)

  if (($EdgesIntegrityX.Count -ne $Index) -or ($EdgesIntegrityY.Count -ne $Index)) {
    Write-Verbose "$($EdgesIntegrityX.Length) -> $Index"
    Write-Verbose "$($EdgesIntegrityY.Length) -> $Index"

    throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
  }
}
Write-Verbose "[State Verified] $($MeasureStateVerification.TotalSeconds)";

$MeasureConnectionDeserialisation = Measure-Command {
  $G = "$($OutputDirectory)\intermediate.connections.g"

  # Read connections information and generate additional edges
  # to represents connections between clusters to add them
  # to global edges arrays
  #

  $Reader = New-Object -TypeName System.IO.StreamReader -ArgumentList $G
  while (!$Reader.EndOfStream) {
      $Items = $Reader.ReadLine().Split();

      $X = $ClustersConfig[$Items[0]];
      $Y = $ClustersConfig[$Items[1]];

      $XO = [System.Random]::Shared.NextInt64(0, $X.Size) + $X.IndexShift;
      $YI = [System.Random]::Shared.NextInt64(0, $Y.Size) + $Y.IndexShift;

      $EdgesX[$EdgesLineIndex] = $XO;
      $EdgesY[$EdgesLineIndex] = $YI;

      $EdgesLineIndex = $EdgesLineIndex + 1;

      $VertexConnectionCounts[$XO] = $VertexConnectionCounts[$XO] + 1;
      $VertexConnectionCounts[$YI] = $VertexConnectionCounts[$YI] + 1;
  }
  $Reader.Close()
};
Write-Verbose "[Connection Deserialization Completed] $($MeasureConnectionDeserialisation.TotalSeconds)";

$MeasureStateVerification = Measure-Command {
  $EdgesIntegrityX = [System.Collections.Generic.HashSet[int]]::new($EdgesX)
  $EdgesIntegrityY = [System.Collections.Generic.HashSet[int]]::new($EdgesY)

  if (($EdgesIntegrityX.Count -ne $Index) -or ($EdgesIntegrityY.Count -ne $Index)) {
    Write-Verbose "$($EdgesIntegrityX.Length) -> $Index"
    Write-Verbose "$($EdgesIntegrityY.Length) -> $Index"

    throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
  }
}
Write-Verbose "[State Verified] $($MeasureStateVerification.TotalSeconds)";

$MeasureCacheEdgePositions = Measure-Command {
  for ($i = 0; $i -lt $VertexConnectionCounts.Length; ++$i) {
    $VertexConnections[$i] = [System.Collections.Generic.List[int]]::new($VertexConnectionCounts[$i]);
  }
  for ($i = 0; $i -lt $EdgesX.Length -and $i -lt $EdgesY.Length; ++$i) {
    $X = $EdgesX[$i];
    $Y = $EdgesY[$i];

    $VertexConnections[$X].Add($i);
    $VertexConnections[$Y].Add($i);
  }
}
Write-Verbose "[Edges Cache Generated] $($MeasureCacheEdgePositions.TotalSeconds)";

# Randomise vertex values by performing random replacements
#
$MeasureShuffle = Measure-Command {
  $ShuffleSet = New-Object System.Collections.Generic.HashSet[int];
  $RandomIterations = [System.Random]::Shared.NextInt64(($Index * 0.20), ($Index * 0.60));
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

    if (($X.IndexShift) -eq ($Y.IndexShift)) {
      continue;
    }

    $ListX = $VertexConnections[$O];
    $ListY = $VertexConnections[$R];

    for ($j = 0; $j -lt $ListX.Count; ++$j) {
      $c  = $ListX[$j];
      if ($EdgesX[$c] -eq $O) { $EdgesX[$c] = $R; } elseif ($EdgesX[$c] -eq $R) { $EdgesX[$c] = $O; }
      if ($EdgesY[$c] -eq $O) { $EdgesY[$c] = $R; } elseif ($EdgesY[$c] -eq $R) { $EdgesY[$c] = $O; }
    }
    for ($j = 0; $j -lt $ListY.Count; ++$j) {
      $c  = $ListY[$j];
      if ($EdgesX[$c] -eq $O) { $EdgesX[$c] = $R; } elseif ($EdgesX[$c] -eq $R) { $EdgesX[$c] = $O; }
      if ($EdgesY[$c] -eq $O) { $EdgesY[$c] = $R; } elseif ($EdgesY[$c] -eq $R) { $EdgesY[$c] = $O; }
    }
  }
}
Write-Verbose "[Shuffle Completed] $($MeasureShuffle.TotalSeconds)";

$MeasureStateVerification = Measure-Command {
  $EdgesIntegrityX = [System.Collections.Generic.HashSet[int]]::new($EdgesX)
  $EdgesIntegrityY = [System.Collections.Generic.HashSet[int]]::new($EdgesY)

  if (($EdgesIntegrityX.Count -ne $Index) -or ($EdgesIntegrityY.Count -ne $Index)) {
    Write-Verbose "$($EdgesIntegrityX.Length) -> $Index"
    Write-Verbose "$($EdgesIntegrityY.Length) -> $Index"

    throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
  }
}
Write-Verbose "[State Verified] $($MeasureStateVerification.TotalSeconds)";

# Collect clusters information
#
$ClustersConfig | ForEach-Object {
  # Iterate over the cluster's edges (from start to end)
  # and collect 'from' vertices into the array
  #
  $Vertices = [System.Collections.Generic.HashSet[int]]::new();
  for ($i = $_.StartIndex; $i -lt $_.EndIndex; ++$i) {
    $Vertices.Add($EdgesX[$i]) | Out-Null;
  }

  $intermediate = [int[]]::new($Vertices.Count);
  $Vertices.CopyTo($intermediate);

  $Vertices = $intermediate;

  if ($Vertices.Length -ne $_.Size) {
    throw 'There is a mistake in the collecting information about cluster''s edges';
  }

  $_.Vertices = $Vertices;
};

# Output Code
#
$OutputCode = "$($OutputDirectory)\$($ClustersIndex)-$($ClustersConfig.Length)-$($ConnectionEdgePercentage)";

# Print Edges
#
for ($i = 0; $i -lt $EdgesX.Length -and $i -lt $EdgesY.Length; ++$i) {
  $X = $EdgesX[$i];
  $Y = $EdgesY[$i];

  $W = [System.Random]::Shared.NextInt64($MinWeight, $MaxWeight);
  $EdgesPrint[$i] = "$($X) $($Y) $W"
}
Set-Content -Path "$($OutputDirectory)\$($OutputCode).g" -Value $EdgesPrint;

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
Set-Content -Path "$($OutputDirectory)\$($OutputCode).communities.g" -Value $ClustersOutput;

# Perform analysis
#
$MeasureAnalysis = Measure-Command {
  & $GraphAnalysersExecutable `
    -o "$($OutputDirectory)\$($OutputCode).analysis.g" `
    -g "$($OutputDirectory)\$($OutputCode).g" `
    -G weightlist `
    -c "$($OutputDirectory)\$($OutputCode).communities.g" `
    -C rlang `
    2> $null 1> $null;
}
Write-Verbose "[Analysis Generation Completed] $($MeasureAnalysis.TotalSeconds)";

$ClustersConfig | ForEach-Object {
  Remove-Item -Path "$($OutputDirectory)\intermediate.$($_.Code).g";
}
Remove-Item -Path "$($OutputDirectory)\intermediate.connections.g";
