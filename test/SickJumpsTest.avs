MakeTestClip(100000, 1920, 1080, 60)
SickJumps(100, 10000, 2, 8, 5, 5)



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
