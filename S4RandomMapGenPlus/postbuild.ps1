echo "Calling Postbuild Script"
$startEditor = $true
echo $args[0]
Copy-Item $args[0] "D:\Ubisoft Games\thesettlers4\Editor\MapGenerator2.dll"
if($startEditor) {
	Start-Process -FilePath "D:\Ubisoft Games\thesettlers4\Editor\S4Editor+_203.exe" -WorkingDirectory "D:\Ubisoft Games\thesettlers4\Editor\"
}