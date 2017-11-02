MakeTestClip(1000, 1280, 720, 60)
SickJumps(100, 900, 1, 4, 2, 2, script_variable="debug")
ShowMultiplier()

function MakeTestClip(int length, int width, int height, float fps)
{
  length = Default(length, 240)
  width = Default(width, 640)
  height = Default(height, 480)
  fps = Default(fps, 24.0)
  BlankClip(length, width, height, fps=fps)
  AudioDub(last, Tone(length=length/fps,level=0.125))
  ScriptClip( \
    """
    Subtitle(String(current_frame), x=-1, y=-1, align=5, size=256.0, font="Segoe WP", halo_color=$FF000000, text_color=$FFFFFF)
    """)
}

function ShowMultiplier(clip c, string "format_string")
{
  format_string = Default(format_string, """"x%1.1f"""")
  
  c
  
  # Remember that after_frame=true is key, to make sure the script variable both
  # exists and is set properly each frame.
  ScriptClip( \
  \
  """
  debug = debug.MidStr(debug.FindStr("frame:") + 6)
  input_frame_number = debug.LeftStr(debug.FindStr(":") - 1)
  debug = debug.MidStr(debug.FindStr("multiplier:") + 11)
  multiplier = debug.LeftStr(debug.FindStr(":") - 1)
  debug = debug.MidStr(debug.FindStr("section:") + 8)
  section = debug
  """ + \
  \
  "
  multiplier_float = Value(multiplier)
  multiplier_formatted = String(multiplier_float, " + format_string + ")
  " + \
  \
  """
  Subtitle(multiplier_formatted, x=last.Width() - 48, y=last.Height() - 8, align=3, size=256.0, font="Segoe WP", halo_color=$FF000000, text_color=$FFFFFF, lsp=10)
  """, \
  \
  after_frame=true)

  return last
}


