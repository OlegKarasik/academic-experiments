# [CmdletBinding()]
# param(
#   [ValidateNotNullOrEmpty()]
#   [string] $ToolsDirectory = $(throw 'ToolsDirectory parameter is required'),
#   [ValidateNotNullOrEmpty()]
#   [string] $OutputFile = $(throw 'OutputFile parameter is required.')
# )

$ToolsDirectory = 'C:\Users\Administrator\Projects\GitHub\academic-experiments\src\tools\build\mingw-gcc-release'
$TempDirectory = "$($ToolsDirectory)"


$GraphGeneratorsExecutable = "$($ToolsDirectory)\graphg.exe";



if (-not (Test-Path -Path $ToolsDirectory -PathType Container -ErrorAction Stop)) {
  throw "Directory '$ToolsDirectory' does not exist";
};

$Clusters = @(
  @{
    Size = 40;
    Type = 1; #Complete
  },
  @{
    Size = 50;
    Type = 1; #Complete
  },
  @{
    Size = 30;
    Type = 1; #Complete
  },
  @{
    Size = 80;
    Type = 1; #Complete
  },
  @{
    Size = 20;
    Type = 1; #Complete
  }
);

# ^
# Input

# Pre-prcoess the input by generating random codes
# and assigning start and end indexes
#
$ClustersCode = 0;
$ClustersIndex = 0;
$Clusters | ForEach-Object {
  $_.Code       = $ClustersCode;
  $_.StartIndex = $ClustersIndex;
  $_.EndIndex   = $ClustersIndex + $_.Size;

  $ClustersCode  += 1;
  $ClustersIndex += $_.Size;
};

# Generate cluster.* files based on the input information and
# read all vertices into global vertices array
#
$Index    = 0;
$Vertices = @();

$Clusters | ForEach-Object {
  $C = $_;

  $C.VerticesStartIndex = $Vertices.Length;

  & $GraphGeneratorsExecutable `
    -o "$($TempDirectory)\cluster.$($C.Code).g" `
    -O edgelist `
    -a "$($C.Type)" `
    -v "$($C.Size)"

  Get-Content -Path "$($TempDirectory)\cluster.$($C.Code).g" | ForEach-Object {
    $Success = $_ -match '(?<X>\d+)\s(?<Y>\d+)';
    if ($Success -eq $false) {
      throw 'There is a mistake in the generated graph, the format doesn''t match edgelist';
    }

    $X = [int]::Parse($Matches['X']) + $Index;
    $Y = [int]::Parse($Matches['Y']) + $Index;

    $Vertices += @{ X = $X; Y = $Y; };
  }
  $Index = $Index + $C.Size;

  $C.VerticesEndIndex = $Vertices.Length;
};

# Generate a random connected graph to represent
# connections between clusters (by generating the connected graph)
#
& $GraphGeneratorsExecutable `
    -o "$($TempDirectory)\cluster.connections.g" `
    -O edgelist `
    -a 2 `
    -v "$($Clusters.Length)" `
    -e $((($Clusters.Length) * ($Clusters.Length - 1) / 2))

# Read connections information and generate additional edges
# to represents connections between clusters to add them
# to global vertices array
#
Get-Content -Path "$($TempDirectory)\cluster.connections.g" | ForEach-Object {
  $Success = $_ -match '(?<X>\d+)\s(?<Y>\d+)';
  if ($Success -eq $false) {
    throw 'There is a mistake in the generated connections graph, the format doesn''t match edgelist';
  }

  $X = $Clusters[[int]::Parse($Matches['X'])];
  $Y = $Clusters[[int]::Parse($Matches['Y'])];

  $XO = [System.Random]::Shared.NextInt64(0, $X.Size - 1) + $X.StartIndex;
  $YI = [System.Random]::Shared.NextInt64(0, $Y.Size - 1) + $Y.StartIndex;

  $Vertices += @{ X = $XO; Y = $YI; }

  $YO = [System.Random]::Shared.NextInt64(0, $Y.Size - 1) + $Y.StartIndex;
  $XI = [System.Random]::Shared.NextInt64(0, $X.Size - 1) + $X.StartIndex;

  $Vertices += @{ X = $YO; Y = $XI; }
}

# Randomise vertex values by performing random replacements
#
$RandomIterations = [System.Random]::Shared.NextInt64($Index / 2, $Index);
for ($i = 0; $i -lt $RandomIterations; ++$i) {
  $O = [System.Random]::Shared.NextInt64(0, $Index);
  $R = [System.Random]::Shared.NextInt64(0, $Index);

  $Vertices | % {
    if ($_.X -eq $O) { $_.X = $R; } elseif ($_.X -eq $R) { $_.X = $O; }
    if ($_.Y -eq $O) { $_.Y = $R; } elseif ($_.Y -eq $R) { $_.Y = $O; }
  };
}

# Collect clusters information
#
$Clusters | ForEach-Object {
  $Values = @();

  for ($i = $_.VerticesStartIndex; $i -lt $_.VerticesEndIndex; ++$i) {
    $Values += $Vertices[$i].X;
  }

  $Values = $Values | Sort-Object | Get-Unique;

  if ($Values.Length -ne $_.Size) {
    throw 'There is a mistake in the collecting information about cluster''s edges';
  }

  $_.Vertices = $Values;
};

# Print results
#
$ClustersOutput      = @();
$ClustersOutputIndex = 1;
$Clusters | ForEach-Object {
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

$VerticesOutput = @();
$Vertices | % {
  $VerticesOutput += "$($_.X) $($_.Y)"
};

Set-Content -Path "$($TempDirectory)\ClustersOutput.g" -Value $ClustersOutput;
Set-Content -Path "$($TempDirectory)\VerticesOutput.g" -Value $VerticesOutput;



