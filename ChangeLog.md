This file only contains the major changes of the IEM Plug-in Suite.
For a list of all changes and bugfixes see the git log.

### Untagged changes
- new plug-ins:
    - **Matrix**Multiplicator
    - **Simple**Decoder
    - **Tool**Box

- general changes
    - added presets (decoders,...)  
    - some GUIs now resizable  
    - smaller GUI layout changes (e.g. ComboBox PopupMenu)

- plugin specific changes
    - MultiEncoder
        - maximum input channel size increased to 64 
        - parameters and colours are stored correctly now
        - MasterPanner is now controllable with mouse wheel (and modifiers)
        - GUI resizable
    - OmniCompressor and DirectionalCompressor
        -  new compressor engine
        -  added 'knee' parameter
        -  can be used as limiters now
    - FdnReverb
        -  added a visual T60 recommendation
        -  maximum reverberation time reduced
        -  filters can now have positive gain
        -  layout restructured, resizable
    -  RoomEncoder
        -  added floor reflections (now up to 236 reflections possible)
        -  default number of reflections set to 19 (chosen arbitrarily)
    -  StereoEncoder
        -  added hidden high-quality mode  