
$git_info = $(git describe --always --long --dirty --abbrev=10 --tags)
$git_short_hash = $(git rev-parse --short HEAD)
$git_hash = $(git rev-parse HEAD)
$date_info = $([String]::Format("{0:yyyy-MM-ddTHH:mm:ss.fffZ}", $(Get-Date).ToUniversalTime()))

Write-Output "#ifndef __BUILD_INFO_H__"
Write-Output "#define __BUILD_INFO_H__"
Write-Output ""
Write-Output "#define BUILD_INFO ""$git_info"""
Write-Output "#define BUILD_INFO_LEN $($git_info.Length)"
Write-Output "#define BUILD_HASH ""$git_hash"""
Write-Output "#define BUILD_HASH_LEN $($git_hash.Length)"
Write-Output "#define BUILD_SHORT_HASH ""$git_short_hash"""
Write-Output "#define BUILD_SHORT_HASH_LEN $($git_short_hash.Length)"
Write-Output "#define BUILD_DATE ""$date_info"""
Write-Output "#define BUILD_DATE_LEN $($date_info.Length)"
Write-Output ""
Write-Output "#endif // __BUILD_INFO_H__"

