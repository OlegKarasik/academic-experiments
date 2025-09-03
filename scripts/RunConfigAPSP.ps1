[CmdletBinding()]
param(
  [string] $GenConfigPath    = $(throw '-GenConfigPath is required'),
  [string] $OutputConfigPath = $(throw '-OutputConfigPath is required')
)

Write-Verbose -Message "GET CONFIG PATH : $GenConfigPath" -ErrorAction Stop;

if (-not (Test-Path -Path $GenConfigPath -PathType Leaf -ErrorAction Stop)) {
  throw "File '$GenConfigPath' does not exist";
};

$GenConfig = Get-Content -Path $GenConfigPath -ErrorAction Stop -Raw | ConvertFrom-Json -ErrorAction Stop;

$ExperimentObjects = [System.Collections.Generic.List[string]]::new();
$GenConfig | ForEach-Object {
  $Versions = $_.Versions;
  $Template = $_.Template;
  $Graphs   = $_.Graphs;

  $GraphObjects = [System.Collections.Generic.List[string]]::new();
  $Graphs | ForEach-Object {
    $Code = $_.Code;
    $Path = $_.Path;

    $Groups = [System.Collections.Generic.Dictionary[string,System.Collections.Generic.List[string]]]::new();
    Get-ChildItem -Path $Path -File `
                | ForEach-Object { return $_.Name } `
                | ForEach-Object {
                  $key = ($_ -split '\.')[0];

                  $values;
                  if (-not $Groups.ContainsKey($key)) {
                    $values = [System.Collections.Generic.List[string]]::new();
                    $Groups[$key] = $values;
                  } else {
                    $values = $Groups[$key];
                  }
                  $values.Add($_);
                } `
                | Out-Null

    $TemplateContent = Get-Content -Path $Template -Raw -ErrorAction Stop;

    $TemplateContentArray = [System.Collections.Generic.List[string]]::new();
    $Groups.Keys | ForEach-Object {
      $key     = $_;
      $values  = $Groups[$key];
      $content = $TemplateContent.ToString();

      $values | ForEach-Object {
        $value             = $_;
        $replacement_key   = "%$($_ -replace $key,'')%";
        $replacement_value = $(Join-Path -Path $Path -ChildPath $value | ConvertTo-Json).Trim('"');
        $content           = $content -replace $replacement_key,$replacement_value;
      }
      $content = $content -replace '%key%',$key;

      $TemplateContentArray.Add($content);
    }

    $GraphObject = "{
      `"Code`": `"$Code`",
      `"Args`": [ $($TemplateContentArray -join ',') ]
    }";
    $GraphObjects.Add($GraphObject);
  }

  $ExperimentVersions = $Versions | ForEach-Object { return "`"$_`"" };
  $ExperimentObject = "{
      `"Versions`": [
        $($ExperimentVersions -join ',')
      ],
      `"Graphs`": [
        $($GraphObjects -join ',')
      ]
    }"
  $ExperimentObjects.Add($ExperimentObject);
}

$FinalConfig = "[ $($ExperimentObjects -join,',') ]";
$Json = ConvertFrom-Json -InputObject $FinalConfig -ErrorAction Stop;

Set-Content -Path $OutputConfigPath -Value $($Json | ConvertTo-Json -Depth 100) -ErrorAction Stop;
