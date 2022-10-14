$outlook = new-object -com Outlook.Application
$session = $outlook.Session
$session.Logon()
$inbox = $outlook.session.GetDefaultFolder(6)
[array]$unreadCount = @(%{$inbox.Items | where {$_.UnRead}}).Count
Write-Host $unreadCount
[array]$ItemCount = @(%{$inbox.Items }).Count
Write-Host $ItemCount

cd 'C:\Program Files\mosquitto\'
./mosquitto_pub.exe -t "unreadmail" -h 192.168.1.136 -m $unreadCount
