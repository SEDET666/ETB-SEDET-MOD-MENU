Write-Output "SEMOD_EX has been discontinued. "
Start-Sleep -s 3
exit

Invoke-WebRequest -Uri "https://github.com/SEDET666/ETB-SEDET-MOD-MENU/raw/refs/heads/main/Quick/SEMOD_EX.zip" -OutFile "$env:TEMP\semodex.zip"
Expand-Archive -Path "$env:TEMP\semodex.zip" -DestinationPath "C:\SEMOD_EX"
