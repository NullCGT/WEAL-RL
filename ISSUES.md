# High Severity
- The game will very occasionally segfault on startup or when creating a new
  map.
- The game will segfault if the user attempts to load a nonexistent xml file
  for map generation purposes.
- The '>' key does not work in the opengl window port.

# Low Severity
- If the map is smaller than the map window and the player moves to the
  far left, odd scrolling behavior will occur. This is not a bug and does
  not impact gameplay, but it does look a bit odd.
- When the full message log is closed in opengl mode, the message window will
  display an extra line for the next turn.