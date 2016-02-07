$s = 0
$e = 0

Function Compile ($filter, $type, $profile, $glProfile) {
	foreach ($file in (ls -Path $dir -Filter $filter)) {
		$path = $file | Resolve-Path -Relative
        $outname = [io.path]::ChangeExtension($file.Name, "bin")
		Write-Output ("Compiling {0}..." -f $path)
		&..\..\..\Tools\shaderc.exe --platform linux -p $glProfile --type $type -f "$path" -o ".\bin\glsl\$outname" -i ..\
		if ($LastExitCode -eq 0) { $Global:s++ } Else { $Global:e++ }
        &..\..\..\Tools\shaderc.exe --platform windows -p $profile -O 3 --type $type -f "$path" -o ".\bin\dx11\$outname" -i ..\
		if ($LastExitCode -eq 0) { $Global:s++ } Else { $Global:e++ }
	}
}

foreach ($dir in (ls -Directory)) {
	Compile "vs_*.sc" "vertex" "vs_4_0" "120"
	Compile "fs_*.sc" "fragment" "ps_4_0" "120"
    Compile "cs_*.sc" "compute" "cs_5_0" "430"
}

Write-Output ""
Write-Output ("Shader Build: {0} succeeded, {1} failed" -f $s,$e)