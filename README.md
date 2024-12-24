Simple audio compressor vst plugin built with the JUCE framework in C++.

## Features

- **Threshold**: Set the level above which the signal will be compressed, ranging from -60dB to 0dB.
- **Ratio**: Control the amount of compression applied to the signal above the threshold, ranging from 1:1 to 20:1.
- **Attack**: Adjust the time it takes for the compressor to start compressing after the signal exceeds the threshold, ranging from 0ms to 100ms.
- **Release**: Set the time it takes for the compressor to stop compressing after the signal falls below the threshold, ranging from 10ms to 1000ms.
- **Makeup Gain**: Apply gain to the compressed signal to compensate for the reduction in level, ranging from 0dB to 24dB.
- **Input**: Control the input gain of the signal before compression, ranging from -24dB to 24dB.

<img width="600" alt="image" src="https://github.com/tylerguest/oneComp/assets/83128087/620b3891-c16d-4bd7-929d-0701d96df12e">

