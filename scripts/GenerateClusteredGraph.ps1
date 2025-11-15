[CmdletBinding()]
param(
  [ValidateNotNullOrEmpty()]
  [string] $ClustersConfigPath         = $(throw './clusters.clusters-config'),
  [ValidateNotNullOrEmpty()]
  [string] $ConnectionEdgePercentage   = $(throw '-ConnectionEdgePercentage parameter is required'),
  [ValidateNotNullOrEmpty()]
  [string] $ConnectionVertexPercentage = $(throw '-ConnectionVertexPercentage parameter is required'),
  [ValidateNotNullOrEmpty()]
  [string] $ConnectionVertexBalance    = $(throw 'ConnectionVertexBalance parameter is required'),
  [ValidateNotNullOrEmpty()]
  [string] $OutputDirectory            = $(throw '-OutputDirectory parameter is required'),
  [string] $ToolsDirectory             = '',
  [int]    $MinWeight                  = 1,
  [int]    $MaxWeight                  = 100
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
Write-Verbose -Message "CONNECTION VERTEX BALANCE    : $ConnectionVertexBalance" -ErrorAction Stop;

$ClustersConfig = Get-Content -Path $ClustersConfigPath -ErrorAction Stop -Raw `
                | ConvertFrom-Json -ErrorAction Stop -AsHashtable;

# Pre-process the input by generating random codes
# and assigning start and end indexes
#
$ClustersCode = 0;
$ClustersIndex = 0;
$ClustersConfig | ForEach-Object {
  $_.Code       = $ClustersCode;
  $_.IndexShift = $ClustersIndex;

  $ClustersCode  += 1;
  $ClustersIndex += $_.Size;
} -ErrorAction Stop;

# Processing input related to balance of input and output vertices
#
function ParseConnectionVertexBalance {
  param (
    [string] $value
  )
  $split = $value -split '/';

  if ($split.Length -ne 2) {
    throw "The '$split' value is invalid and must be either '' or '<in>/<out>' format"
  }

  return @{
    i = $split[0]
    o = $split[1]
  };
};

function CalculateConnectionVertexCount {
  param(
    [int] $max,
    [int] $percent
  )
  return [int](($max * $percent) / 100);
}

function CalculateConnectionVertexBalanceCount {
  param(
    [int] $count,
    [string] $balance
  )

  function GetBalanceValue {
    param(
      [string] $value
    )
    return [int]::Parse($value);
  }

  return [int](($count * $(GetBalanceValue $balance)) / 100);
}

# Prepare individual clusters configuration
#
$ClustersConfig | ForEach-Object {
  $balance = ParseConnectionVertexBalance $ConnectionVertexBalance;

  $_.MinimumBridgeVertices       = CalculateConnectionVertexCount        $_.Size                  $ConnectionVertexPercentage;
  $_.MinimumInputBridgeVertices  = CalculateConnectionVertexBalanceCount $_.MinimumBridgeVertices $balance.i;
  $_.MinimumOutputBridgeVertices = CalculateConnectionVertexBalanceCount $_.MinimumBridgeVertices $balance.o;
} -ErrorAction Stop;

[int] $Index      = 0;
[int] $EdgesCount = 0;

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

    $lines_count = 0;
    switch -File "$($OutputDirectory)\intermediate.$($C.Code).g" { default { ++$lines_count } }

    # Set start boundry
    #
    $C.StartIndex = $EdgesCount;

    $EdgesCount = $EdgesCount + $lines_count;

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
    -v $ClustersConfig.Length `
    -e $ConnectionEdgePercentage `
    2> $null 1> $null;

  $MaximumBridgeVertices = $($ClustersConfig | Measure-Object -Property MinimumBridgeVertices -Maximum).Maximum
  $EdgesCount = $EdgesCount + ($MaximumBridgeVertices * $MaximumBridgeVertices * 2) * $(1..($ClustersConfig.Length - 1) | Measure-Object -Sum).Sum;
} -ErrorAction Stop;
Write-Verbose "[Clusters Generation Completed] $($MeasureClustersGeneration.TotalSeconds)" -ErrorAction Stop;

$EdgesX     = [int[]]::new($EdgesCount);
$EdgesY     = [int[]]::new($EdgesCount);

$EdgesLineIndex  = 0;

$MeasureClustersDeserialisation = Measure-Command {
  $ClustersConfig | ForEach-Object {
    $clusters_config = $_;

    $stream = New-Object -TypeName System.IO.StreamReader -ArgumentList "$($OutputDirectory)\intermediate.$($clusters_config.Code).g" -ErrorAction Stop;
    while (!$stream.EndOfStream) {
        $split_items = $stream.ReadLine().Split();

        $__X = [int]::Parse($split_items[0]) + $clusters_config.IndexShift;
        $__Y = [int]::Parse($split_items[1]) + $clusters_config.IndexShift;

        $EdgesX[$EdgesLineIndex] = $__X;
        $EdgesY[$EdgesLineIndex] = $__Y;

        $EdgesLineIndex = $EdgesLineIndex + 1;
    }
    $stream.Close()

    $Index = $Index + $clusters_config.Size;
  };
} -ErrorAction Stop;
Write-Verbose "[Clusters Deserialisation Completed] $($MeasureClustersDeserialisation.TotalSeconds)";

$MeasureStateVerification = Measure-Command {
  $EdgesIntegrityX = [System.Collections.Generic.HashSet[int]]::new($EdgesX)
  $EdgesIntegrityY = [System.Collections.Generic.HashSet[int]]::new($EdgesY)

  if (($EdgesIntegrityX.Count -ne $Index) -or ($EdgesIntegrityY.Count -ne $Index)) {
    Write-Verbose "$($EdgesIntegrityX.Length) -> $Index" -ErrorAction Stop;
    Write-Verbose "$($EdgesIntegrityY.Length) -> $Index" -ErrorAction Stop;

    throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
  }
} -ErrorAction Stop;
Write-Verbose "[State Verified] $($MeasureStateVerification.TotalSeconds)" -ErrorAction Stop;

$MeasureConnectionDeserialisation = Measure-Command {
  # Randomly select vertices from each cluster as 'bridge' vertices to
  # later generate edges between them depending on connections information
  # from the file
  #
  $bridge_vertices = [int[][]]::new($ClustersConfig.Length);
  $bridge_input_vertices = [int[][]]::new($ClustersConfig.Length);
  $bridge_output_vertices = [int[][]]::new($ClustersConfig.Length);
  for ($i = 0; $i -lt $ClustersConfig.Length; ++$i) {
    $clusters_config            = $ClustersConfig[$i];
    $bridge_vertices[$i]        = [int[]]::new($clusters_config.MinimumBridgeVertices);
    $bridge_input_vertices[$i]  = [int[]]::new($clusters_config.MinimumInputBridgeVertices);
    $bridge_output_vertices[$i] = [int[]]::new($clusters_config.MinimumOutputBridgeVertices);
  }

  for ($i = 0; $i -lt $ClustersConfig.Length; ++$i) {
    $clusters_config     = $ClustersConfig[$i];
    $bridge_vertices_set = [System.Collections.Generic.HashSet[int]]::new($clusters_config.MinimumBridgeVertices);

    while ($bridge_vertices_set.Count -ne $clusters_config.MinimumBridgeVertices) {
      $bridge_vertices_set.Add([System.Random]::Shared.NextInt64(0, $clusters_config.Size));
    }

    $bridge_vertices_set.CopyTo($bridge_vertices[$i]);
  }

  if ($ConnectionVertexSymmetric -eq $true) {
    for ($i = 0; $i -lt $ClustersConfig.Length; ++$i) {
      $clusters_config = $ClustersConfig[$i];
      for ($j = 0; $j -lt $clusters_config.MinimumInputBridgeVertices; ++$j) {
        $vertex = $bridge_vertices[$i][$j];

        $bridge_input_vertices[$i][$j] = $vertex;
        $bridge_output_vertices[$i][$j] = $vertex;
      }
    }
  } else {
    for ($i = 0; $i -lt $ClustersConfig.Length; ++$i) {
      $clusters_config = $ClustersConfig[$i];
      for ($j = 0; $j -lt $clusters_config.MinimumInputBridgeVertices; ++$j) {
        $bridge_input_vertices[$i][$j] = $bridge_vertices[$i][$j];
      }
    }
    for ($i = 0; $i -lt $ClustersConfig.Length; ++$i) {
      $clusters_config = $ClustersConfig[$i];
      for ($j = 0; $j -lt $clusters_config.MinimumOutputBridgeVertices; ++$j) {
        $bridge_output_vertices[$i][$j] = $bridge_vertices[$i][$clusters_config.MinimumInputBridgeVertices + $j];
      }
    }
  }

  $lines_count = 0;
  switch -File "$($OutputDirectory)\intermediate.connections.g" { default { ++$lines_count } }

  $x_connections = [int[]]::new($lines_count);
  $y_connections = [int[]]::new($lines_count);

  $line_index = 0;
  $stream = New-Object -TypeName System.IO.StreamReader -ArgumentList "$($OutputDirectory)\intermediate.connections.g" -ErrorAction Stop;
  while (!$stream.EndOfStream) {
      $split_items = $stream.ReadLine().Split();

      $x_connections[$line_index] = [int]::Parse($split_items[0]);
      $y_connections[$line_index] = [int]::Parse($split_items[1]);

      $line_index++;
  }
  $stream.Close();

  $clusters_connection_track = [System.Collections.Generic.HashSet[string]]::new();

  for ($i = 0; $i -lt $lines_count; ++$i) {
    $x_clusters_index = $x_connections[$i];
    $y_clusters_index = $y_connections[$i];

    if ($clusters_connection_track.Add("$($x_clusters_index)/$($y_clusters_index)") -eq $false) {
      continue;
    }

    $x_clusters_config = $ClustersConfig[$x_clusters_index];
    $y_clusters_config = $ClustersConfig[$y_clusters_index];

    $x_bridge_input_vertices = $bridge_input_vertices[$x_clusters_index];
    $y_bridge_input_vertices = $bridge_input_vertices[$y_clusters_index];

    $x_bridge_output_vertices = $bridge_output_vertices[$x_clusters_index];
    $y_bridge_output_vertices = $bridge_output_vertices[$y_clusters_index];

    $x_bridge_output_vertices | ForEach-Object {
      $x_output = $_;

      $y_bridge_input_vertices | ForEach-Object {
        $y_input = $_;

        $EdgesX[$EdgesLineIndex] = $x_output + $x_clusters_config.IndexShift;
        $EdgesY[$EdgesLineIndex] = $y_input  + $y_clusters_config.IndexShift;

        $EdgesLineIndex = $EdgesLineIndex + 1;
      } -ErrorAction Stop;
    } -ErrorAction Stop;
    $y_bridge_output_vertices | ForEach-Object {
      $y_output = $_;

      $x_bridge_input_vertices | ForEach-Object {
        $x_input = $_;

        $EdgesX[$EdgesLineIndex] = $y_output + $y_clusters_config.IndexShift;
        $EdgesY[$EdgesLineIndex] = $x_input  + $x_clusters_config.IndexShift;

        $EdgesLineIndex = $EdgesLineIndex + 1;
      } -ErrorAction Stop;
    } -ErrorAction Stop;
  }
} -ErrorAction Stop;
Write-Verbose "[Connection Deserialization Completed] $($MeasureConnectionDeserialisation.TotalSeconds)" -ErrorAction Stop;

$MeasureStateVerification = Measure-Command {
  $EdgesIntegrityX = [System.Collections.Generic.HashSet[int]]::new($EdgesX)
  $EdgesIntegrityY = [System.Collections.Generic.HashSet[int]]::new($EdgesY)

  if (($EdgesIntegrityX.Count -ne $Index) -or ($EdgesIntegrityY.Count -ne $Index)) {
    Write-Verbose "$($EdgesIntegrityX.Count) -> $Index" -ErrorAction Stop;
    Write-Verbose "$($EdgesIntegrityY.Count) -> $Index" -ErrorAction Stop;

    throw 'There is a mistake in the edges generation. The number of unique vertices doesn''t match';
  }
} -ErrorAction Stop;
Write-Verbose "[State Verified] $($MeasureStateVerification.TotalSeconds)" -ErrorAction Stop;

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
} -ErrorAction Stop;

# Output Code
#
$OutputCode = "$($ClustersIndex)-$($ClustersConfig.Length)-$($ConnectionEdgePercentage)-$($ConnectionVertexPercentage)";

# Print Edges
#
$MeasurePrintEdges = Measure-Command {
  $EdgesPrint = [string[]]::new($EdgesLineIndex);
  for ($i = 0; $i -lt $EdgesLineIndex; ++$i) {
    $X = $EdgesX[$i];
    $Y = $EdgesY[$i];

    $W = [System.Random]::Shared.NextInt64($MinWeight, $MaxWeight);
    $EdgesPrint[$i] = "$($X) $($Y) $W"
  }
  Set-Content -Path "$($OutputDirectory)\$($OutputCode).g" -Value $EdgesPrint -ErrorAction Stop;
} -ErrorAction Stop;
Write-Verbose "[Print Edges Completed] $($MeasurePrintEdges.TotalSeconds)" -ErrorAction Stop;

# Print Clusters
#
$MeasurePrintClusters = Measure-Command {
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
  Set-Content -Path "$($OutputDirectory)\$($OutputCode).communities.g" -Value $ClustersOutput -ErrorAction Stop;
} -ErrorAction Stop;
Write-Verbose "[Print Clusters Completed] $($MeasurePrintClusters.TotalSeconds)" -ErrorAction Stop;

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
} -ErrorAction Stop;
Write-Verbose "[Analysis Generation Completed] $($MeasureAnalysis.TotalSeconds)" -ErrorAction Stop;

$ClustersConfig | ForEach-Object {
  Remove-Item -Path "$($OutputDirectory)\intermediate.$($_.Code).g" -ErrorAction Stop;
} -ErrorAction Stop;
Remove-Item -Path "$($OutputDirectory)\intermediate.connections.g" -ErrorAction Stop;
