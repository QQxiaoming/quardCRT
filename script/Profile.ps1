$Global:__LastHistoryId = -1

function Global:__Terminal-Get-LastExitCode {
  if ($? -eq $True) {
    return 0
  }
  if ("$LastExitCode" -ne "") { return $LastExitCode }
  return -1
}

function prompt {

  # First, emit a mark for the _end_ of the previous command.

  $gle = $(__Terminal-Get-LastExitCode);
  $LastHistoryEntry = $(Get-History -Count 1)
  # Skip finishing the command if the first command has not yet started
  if ($Global:__LastHistoryId -ne -1) {
    if ($LastHistoryEntry.Id -eq $Global:__LastHistoryId) {
      # Don't provide a command line or exit code if there was no history entry (eg. ctrl+c, enter on no command)
      $out += "`e]133;D`a"
    } else {
      $out += "`e]133;D;$gle`a"
    }
  }


  $loc = $($executionContext.SessionState.Path.CurrentLocation);
  $hostname = $env:computername | Select-Object
  $username = $env:username | Select-Object

  # Prompt started
  $out += "`e]133;A`a";

  # CWD
  $out += "`e]2;";
  $out += "$username";
  $out += "@";
  $out += "$hostname";
  $out += ":";
  $out += "$loc`a";

  # (your prompt here)
  $out += "PS $loc$('>' * ($nestedPromptLevel + 1)) ";

  # Prompt ended, Command started
  $out += "`e]133;B`a";

  $Global:__LastHistoryId = $LastHistoryEntry.Id

  return $out
}
