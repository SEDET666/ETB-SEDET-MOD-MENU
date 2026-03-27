Invoke-WebRequest -Uri "https://gitee.com/sedet/SEMOD-CheckUpdate/raw/master/QuickIns.zip" -OutFile "$env:TEMP\semodq.zip"
Expand-Archive -Path "$env:TEMP\semodq.zip" -DestinationPath "$env:TEMP\semod_q"
Invoke-WebRequest -Uri "https://gitee.com/sedet/SEMOD-CheckUpdate/raw/master/SEMODG_P.pak" -OutFile "$env:TEMP\semod_q\SEMOD_P.pak"
& "$env:TEMP\semod_q\QuickIns.exe"
