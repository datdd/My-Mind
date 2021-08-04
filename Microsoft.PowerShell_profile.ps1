$env:homedir = $HOME

# get the last part of path
function get-diralias ([string]$loc) {
    # check if we are in our home script dir
    # in that case return grave sign
    if ($loc.Equals($env:homedir)) {
        return "~"
    }

    return (Split-Path (Get-Location) -Leaf)
}

# Set prompt
function prompt {
	$Host.UI.RawUI.WindowTitle = "$env:UserName@: $(get-diralias($(get-location)))"
	
	Write-Host '[' -NoNewline
	Write-Host "$env:UserName@$env:ComputerName : $(get-diralias($(get-location)))" -ForegroundColor Blue -NoNewline
	Write-Host ']' -NoNewline
	
	Write-Host '[' -NoNewline
	Write-Host (Get-Date -format 'hh:mm:ss tt') -ForegroundColor Green -NoNewline
	Write-Host ']' -NoNewline

	return "$ "
}
