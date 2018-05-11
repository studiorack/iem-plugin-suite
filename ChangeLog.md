This file only contains the major changes of the IEM Plug-in Suite.
For a list of all changes and bugfixes see the git log.

Please note, that breaking changes are marked with `!!BREAKING CHANGE!!`. They might lead to an unexpected behaviour and might not be compatible with your previous projects without making some adaptions.

## v1.3.0
- general changes
    - unity gain normalization has moved fully to the decoding stage:  (`!!BREAKING CHANGE!!`)
        - encoding a source results in unity gain in the W-channel (omni)
        - decoding is normalized resulting in unity gain for the sampled mono signal
    - renamed **Matrix**Multiplicator to **Matrix**Multiplier (`!!BREAKING CHANGE!!`)
    - refactored SpherePanner, which fixes automation writing issues
    - added linear elevation plot style to SpherePanner (triggered by double-click)

- plug-in specific changes
    - **AllRA**Decoder
        - fixed distorted visualization
        - added visualization of the acos-rE source width
    - **Binaural**Decoder
        - performance improvements
        - new binaural filters
    - **Dual**Delay
        - fixed high CPU load which occured occasionally
    - **Matrix**Multiplier
        - smaller GUI changes
    - **Multi**Encoder
        - colour-chooser now has the same look as the elements in the sphere panner, again
        - Solo/Mute changes will repaint sphere
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
    - **Stereo**Encoder
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
    - **Probe**Decoder
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
    - **DirectionalCompressor**Decoder
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
        - adjusted parameter limits (threshold and makeup gain)  (`!!BREAKING CHANGE!!`)
    - **Directivity**Shaper
        - added SN3D support (`!!BREAKING CHANGE!!`)
        - renamed parameters (probe instead of master)  (`!!BREAKING CHANGE!!`)
        - using azimuth and elevation instead of yaw and pitch  (`!!BREAKING CHANGE!!`)
        - adjusted parameter limits (threshold and makeup gain)  (`!!BREAKING CHANGE!!`)
    - **Room**Shaper
        - added SN3D support (`!!BREAKING CHANGE!!`)
    - **Simple**Decoder
        - new bass-management (`!!BREAKING CHANGE!!`)
        - added warning, showing if subwoofer channel is already occupied

        
## v1.2.0
- new plug-ins:
    - **AllRA**Decoder
    
- plug-in specific changes
    - **Binaural**Decoder
        - refactored convolution -> way better performance on all platforms
    - **Matrix**Multiplicator
        - fixed bug that only input channels up to a square-number count are processed (e.g. 16 instead of 20)

## v1.1.1
- general changes
    - added binaural IO widget
    - directivity IO widget now shows the normalization (which is N3D)
    - added warning sign to titlebar widgets when bus is too small
    
- plug-in specific changes
    - **Multi**Encoder
        - changed element colours are now displayed at once
    - **Binaural**Decoder
        - making use of new binaural IO widget
        - performance improvements on windows and linux
    - **Directivity**Shaper
        - better GUI performance
        - 5dB subgrid for directivity shaper
        - more linear default values for filters
    - **Room**Encoder
        - fixed bug which lead to filter output values way beyond of 1
    - **Tool**Box
        - added adaptor tool for lower order Ambisonic (LOA) signals to be compatible with HOA decoder weights


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

- plug-in specific changes
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
