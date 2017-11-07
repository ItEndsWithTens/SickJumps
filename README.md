
  SickJumps 0.1.0
  ===============

  A speed ramping plugin for [Avisynth](http://www.avisynth.nl).

  ### Requirements ###
  1. Avisynth 2.6 or greater, or Avisynth+
  2. Microsoft's Visual C++ 2015 Redistributable

  robert.martens@gmail.com  
  [@ItEndsWithTens](https://twitter.com/ItEndsWithTens)

  ### Usage ###

    SickJumps(clip c, int "first_frame", int "last_frame",
              float "start_multiplier", float full_multiplier",
              float "up_seconds", float "down_seconds",
              string "script_variable", float "end_multiplier")

  **c** clip
  - The input clip. Can be any colorspace and audio format.

  **first_frame** int, default 0
  - The first frame of the ramp up to full speed.

  **last_frame** int, default c frame count - 1
  - The last frame of the ramp down to the ending speed.

  **start_multiplier** float, default 1.0
  - The speed multiplier to use before the ramp up.

  **full_multiplier** float, default 2.0
  - The speed multiplier to use between ramp up and ramp down.

  **up_seconds** float, default c length / 4.0  
  **down_seconds** float, default c length / 4.0
  - The resulting duration, in seconds, of the ramp up and ramp down.

  **script_variable** string
  - Assigns a string, containing some debug info, to the specified variable  
    name. Said string is colon-delimited, and takes the following form:

    ```"frame:X:multiplier:Y:section:Z"```

    Respectively, it provides the current input frame number, multiplier, and  
    a simple label describing what section ("before", "ramp up", "full speed",  
    "ramp down", or "after") the current output frame resides in.

    The string is updated each frame, and is intended for use with Avisynth's  
    runtime environment. Be sure to set ScriptClip's after_frame to true so  
    the variable gets set properly.

    See the source distribution for an example script that demonstrates one  
    approach to parsing the string and using some of its information.  

  **end_multiplier** float, default start_multiplier
  - The speed multiplier to use after the ramp down.
