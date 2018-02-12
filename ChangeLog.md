This file only contains the major changes of the IEM Plug-in Suite.
For a list of all changes and bugfixes see the git log.

## untagged changes
- general changes
    - added binaural IO widget
- plugin specific changes
    - **Multi**Encoder
        - changed element colours are now displayed at once
    - **Binaural**Decoder
        - making use of new binaural IO widget
    - **Directivity**Shaper
        - better GUI performance
        - 5dB subgrid for directivity shaper
        - more linear default values for filters
    - **Room**Encoder
        - fixed bug which lead to filter output values way beyond of 1


## v1.1.0
- new plug-ins:
    - **Matrix**Multiplicator
    - **Simple**Decoder
    - **Tool**Box
    - **Binaural**Decoder

- general changes
    - switched to JUCE develop branch !!
    - added configurations (decoders,...)  
    - some GUIs now resizable  
    - smaller GUI layout changes (e.g. ComboBox PopupMenu)

- plugin specific changes
    - **Multi**Encoder
        - maximum input channel size increased to 64 
        - parameters and colours are stored correctly now
        - MasterPanner is now controllable with mouse wheel (and modifiers)
        - GUI resizable
    - **Omni**Compressor and **Directional**Compressor
        -  new compressor engine
        -  added 'knee' parameter
        -  can be used as limiters now
    - **Fdn**Reverb
        -  added a visual T60 recommendation
        -  maximum reverberation time reduced
        -  filters can now have positive gain
        -  layout restructured, resizable
    -  **Room**Encoder
        -  added floor reflections (now up to 236 reflections possible)
        -  default number of reflections set to 19 (chosen arbitrarily)
    -  **Stereo**Encoder
        -  added hidden high-quality mode  
