echo "Calling Prebuild Script"
$p = Get-Process S4Editor+_203
if($p)
{
	Stop-Process -InputObject $p
}