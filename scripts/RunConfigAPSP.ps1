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
    $Code           = $_.Code;
    $Path           = $_.Path;
    $VariableGroups = @( @{ Key = 'default'; Parameters = @{  } } );

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

    $TemplateContentArrayKeys = [System.Collections.Generic.List[string]]::new();
    $TemplateContentArray     = [System.Collections.Generic.List[string]]::new();
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

      $VariableGroups | ForEach-Object {
        $group     = $_;
        $group_key = $group.Key;

        $content = $content -replace '%key%',$group_key;

        $TemplateContentArrayKeys.Add($key);
        $TemplateContentArray.Add($content);
      }
    }

    if ($Code -eq '*') {
      for ($i = 0; $i -lt $Groups.Keys.Count; ++$i) {
        $GraphObject = "{
          `"Code`": `"$($TemplateContentArrayKeys[$i])`",
          `"Args`": [ $($TemplateContentArray[$i]) ]
        }";
        $GraphObjects.Add($GraphObject);
      }
    } else {
      if ($Groups.Keys.Count -ne 1) {
        throw 'Invalid configuration. The arguments cannot be applied to different graphs';
      }
      throw 'Not implemented';
    }
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
