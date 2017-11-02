

  --SickJumps 0.1.0--

      A speed ramping plugin for Avisynth+.

    robert.martens@gmail.com
    @ItEndsWithTens


  --Usage--

    SickJumps(clip c, int "first_frame", int "last_frame",
              float "start_multiplier", float full_multiplier",
              float "up_seconds", float "down_seconds",
              string "script_variable")

    c  clip

      The input clip.

    first_frame  int, default 0

      The first frame of the ramp up to full speed.

    last_frame  int, default c frame count - 1

      The last frame of the ramp back down to the starting speed.

    start_multiplier  float, default 1.0

      The speed multiplier to use outside of the ramped portion of c.

    full_multiplier  float, default 1.0

      The speed multiplier to use as the target within the ramped portion of c.

    up_seconds  float, default c length / 4.0
    down_seconds  float, default c length / 4.0

      The resulting duration, in seconds, of the ramp up and ramp down.

    script_variable  string

      A variable name to which the current multiplier for a frame should be
      assigned. Intended for use with Avisynth's runtime environment. Be sure to
      set ScriptClip's after_frame to true so the variable gets set properly.
