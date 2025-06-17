[CmdletBinding()]
param(
  [int]    $VertexCount       = $(throw '-GraphSize parameter is required'),
  [int]    $ClustersCount     = $(throw '-ClustersCount parameter is required'),
  [int]    $ClustersDeviation = 10,
  [ValidateNotNullOrEmpty()]
  [string] $Output            = $(throw '-Output parameter is required')
)

$PieceBaseline  = [int]($VertexCount   / $ClustersCount);
$PieceDeviation = [int]($PieceBaseline * $ClustersDeviation / 100);
$PieceLow       = $PieceBaseline - $PieceDeviation;
$PieceHigh      = $PieceBaseline + $PieceDeviation;

$Pieces = @();
for ($i = 0; $i -lt $ClustersCount; ++$i) {
  $Pieces += [System.Random]::Shared.NextInt64($PieceLow, $PieceHigh);
}

$Sum = ($Pieces | Measure-Object -Sum).Sum;
if ($Sum -ne $VertexCount) {
  $Value = $Sum -gt $VertexCount ? -1 : 1;
  while ($Sum -ne $VertexCount) {
    $Index = [System.Random]::Shared.NextInt64($ClustersCount);

    $Pieces[$Index] += $Value;
    $Sum            += $Value;
  }
}
$Sum = ($Pieces | Measure-Object -Sum).Sum;
if ($Sum -ne $VertexCount) {
  throw 'Unexpected issue. Looks like compensation routine has failed.';
}

$OutputClusters = @();
for ($i = 0; $i -lt $ClustersCount; ++$i) {
  $OutputClusters += @{ Size = $Pieces[$i]; Type = 1; }
}

Set-Content -Path $Output -Value $(ConvertTo-Json $OutputClusters);
