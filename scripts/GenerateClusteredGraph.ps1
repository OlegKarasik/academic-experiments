[CmdletBinding()]
param(
  [ValidateNotNullOrEmpty()]
  [string] $ClustersConfigPath = $(throw 'ClustersConfigPath parameter is required'),
  [ValidateNotNullOrEmpty()]
  [string] $ToolsDirectory = $(throw 'ToolsDirectory parameter is required'),
  [ValidateNotNullOrEmpty()]
  [string] $OutputDirectory = $(throw 'OutputDirectory parameter is required.')
)

if (-not (Test-Path -Path $ClustersConfigPath -PathType Leaf -ErrorAction Stop)) {
  throw "File '$ClustersConfigPath' does not exist";
};
if (-not (Test-Path -Path $ToolsDirectory -PathType Container -ErrorAction Stop)) {
  throw "Directory '$ToolsDirectory' does not exist";
};
if (-not (Test-Path -Path $OutputDirectory -PathType Container -ErrorAction Stop)) {
  throw "Directory '$OutputDirectory' does not exist";
};

$GraphGeneratorsExecutable = "$($ToolsDirectory)\graphg.exe";

if (-not (Test-Path -Path $GraphGeneratorsExecutable -PathType Leaf -ErrorAction Stop)) {
  throw "File '$GraphGeneratorsExecutable' does not exist";
};

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

$ClustersConfig | ForEach-Object {
  $C = $_;

  # Save the current length of vertices array
  # as 'zero' index of the cluster' edges (in the global array).
  #
  $C.StartIndex = $Edges.Length;

  & $GraphGeneratorsExecutable `
    -o "$($OutputDirectory)\cluster.$($C.Code).g" `
    -O edgelist `
    -a "$($C.Type)" `
    -v "$($C.Size)"

  Get-Content -Path "$($OutputDirectory)\cluster.$($C.Code).g" | ForEach-Object {
    $Success = $_ -match '(?<X>\d+)\s(?<Y>\d+)';
    if ($Success -eq $false) {
      throw 'There is a mistake in the generated graph, the format doesn''t match edgelist';
    }

    $X = [int]::Parse($Matches['X']) + $Index;
    $Y = [int]::Parse($Matches['Y']) + $Index;

    $Edges += @{ X = $X; Y = $Y; };
  }
  $Index = $Index + $C.Size;

  # Save the current length of edges array
  # as the end index of cluster's edges (in the global array)
  #
  $C.EndIndex = $Edges.Length;
};

# Generate a random connected graph to represent
# connections between clusters (by generating the connected graph)
#
& $GraphGeneratorsExecutable `
  -o "$($OutputDirectory)\cluster.connections.g" `
  -O edgelist `
  -a 2 `
  -v "$($ClustersConfig.Length)" `
  -e $((($ClustersConfig.Length) * ($ClustersConfig.Length - 1) / 2))

# Read connections information and generate additional edges
# to represents connections between clusters to add them
# to global edges array
#
Get-Content -Path "$($OutputDirectory)\cluster.connections.g" | ForEach-Object {
  $Success = $_ -match '(?<X>\d+)\s(?<Y>\d+)';
  if ($Success -eq $false) {
    throw 'There is a mistake in the generated connections graph, the format doesn''t match edgelist';
  }

  $X = $ClustersConfig[[int]::Parse($Matches['X'])];
  $Y = $ClustersConfig[[int]::Parse($Matches['Y'])];

  $XO = [System.Random]::Shared.NextInt64(0, $X.Size - 1) + $X.IndexShift;
  $YI = [System.Random]::Shared.NextInt64(0, $Y.Size - 1) + $Y.IndexShift;

  $Edges += @{ X = $XO; Y = $YI; }

  $YO = [System.Random]::Shared.NextInt64(0, $Y.Size - 1) + $Y.IndexShift;
  $XI = [System.Random]::Shared.NextInt64(0, $X.Size - 1) + $X.IndexShift;

  $Edges += @{ X = $YO; Y = $XI; }
}

# Randomise vertex values by performing random replacements
#
$RandomIterations = [System.Random]::Shared.NextInt64($Index / 2, $Index);
for ($i = 0; $i -lt $RandomIterations; ++$i) {
  $O = [System.Random]::Shared.NextInt64(0, $Index - 1);
  $R = [System.Random]::Shared.NextInt64(0, $Index - 1);

  $Edges | ForEach-Object {
    if ($_.X -eq $O) { $_.X = $R; } elseif ($_.X -eq $R) { $_.X = $O; }
    if ($_.Y -eq $O) { $_.Y = $R; } elseif ($_.Y -eq $R) { $_.Y = $O; }
  };
}

# Collect clusters information
#
$ClustersConfig | ForEach-Object {
  $Vertices = @();

  # Iterate over the cluster's edges (from start to end)
  # and collect 'from' vertices into the array
  #
  for ($i = $_.StartIndex; $i -lt $_.EndIndex; ++$i) {
    $Vertices += $Edges[$i].X;
  }

  $Vertices = $Vertices | Sort-Object | Get-Unique;

  if ($Vertices.Length -ne $_.Size) {
    throw 'There is a mistake in the collecting information about cluster''s edges';
  }

  $_.Vertices = $Vertices;
};

# Print results
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

$EdgesOutput = @();
$Edges | ForEach-Object {
  $EdgesOutput += "$($_.X) $($_.Y)"
};

Set-Content -Path "$($OutputDirectory)\ClustersOutput.g" -Value $ClustersOutput;
Set-Content -Path "$($OutputDirectory)\EdgesOutput.g" -Value $EdgesOutput;

$ClustersConfig | ForEach-Object {
  Remove-Item -Path "$($OutputDirectory)\cluster.$($_.Code).g";
}
"$($OutputDirectory)\cluster.connections.g"

