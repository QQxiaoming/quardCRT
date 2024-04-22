$Global:__LastHistoryId = -1

function Global:__Terminal-Get-LastExitCode {
  if ($? -eq $True) {
    return 0
  }
  if ("$LastExitCode" -ne "") { return $LastExitCode }
  return -1
}

function prompt {
  $esc = "$([char]27)"
  $bell = "$([char]7)"
  # First, emit a mark for the _end_ of the previous command.

  $gle = $(__Terminal-Get-LastExitCode);
  $LastHistoryEntry = $(Get-History -Count 1)
  # Skip finishing the command if the first command has not yet started
  if ($Global:__LastHistoryId -ne -1) {
    if ($LastHistoryEntry.Id -eq $Global:__LastHistoryId) {
      # Don't provide a command line or exit code if there was no history entry (eg. ctrl+c, enter on no command)
      $out += "$esc";
      $out += "]133;D";
      $out += "$bell";
    } else {
      $out += "$esc";
      $out += "]133;D;";
      $out += "$gle";
      $out += "$bell";
    }
  }


  $loc = $($executionContext.SessionState.Path.CurrentLocation);
  $hostname = $env:quardcrt_computername | Select-Object
  $username = $env:quardcrt_username | Select-Object

  # Prompt started
  $out += "$esc";
  $out += "]133;A";
  $out += "$bell";

  # CWD
  $out += "$esc";
  $out += "]2;";
  $out += "$username";
  $out += "@";
  $out += "$hostname";
  $out += ":";
  $out += "$loc";
  $out += "$bell";

  # (your prompt here)
  $out += "PS $loc$('>' * ($nestedPromptLevel + 1)) ";

  # Prompt ended, Command started
  $out += "$esc";
  $out += "]133;B";
  $out += "$bell";

  $Global:__LastHistoryId = $LastHistoryEntry.Id

  return $out
}

Set-PSReadLineOption -HistorySearchCursorMovesToEnd
Set-PSReadlineKeyHandler -Key Tab -Function MenuComplete
Set-PSReadlineKeyHandler -Key UpArrow -Function HistorySearchBackward
Set-PSReadlineKeyHandler -Key DownArrow -Function HistorySearchForward
