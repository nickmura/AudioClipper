# AudioClipper


<img width="507" height="548" alt="image" src="https://github.com/user-attachments/assets/cd759815-41a3-4285-a60c-c8bb48016ea8" />


A saturation and clipping plugin for adding warmth, grit, or aggressive distortion to your audio. Built with JUCE.

## What it does

AudioClipper lets you push your audio into different flavors of clipping - from subtle analog-style warmth to hard digital distortion. You get four clipping algorithms to choose from, plus tools like oversampling to keep things clean and stereo processing options to shape your sound.

## Clipping Modes

- **Tanh** - The classic. Smooth, warm saturation that sounds natural and musical. Great for adding thickness without being obvious.
- **Arctan** - Similar vibe to tanh but a bit gentler. Nice for subtle coloring.
- **Cubic** - Soft saturation with its own character. Good middle ground between smooth and aggressive.
- **Hard Clip** - Straight up digital clipping. Flat-tops your signal at the threshold. Use oversampling with this one to tame the aliasing.

## Controls

**Gain Staging**
- **Input Gain** - Drive your signal harder into the clipper (-24 to +24 dB)
- **Output Gain** - Bring it back down after clipping (-24 to +24 dB)
- **Threshold** - Where the clipping kicks in (-24 to 0 dB)
- **Ceiling** - Final output limiter to catch any overs (-12 to 0 dB)
- **Mix** - Blend dry and wet signals (0-100%)

**Quality**
- **Oversampling** - 1x, 2x, or 4x. Higher = cleaner but more CPU. Definitely use 2x or 4x with hard clipping.

**Stereo Options**
- **Stereo Link** - Makes clipping decisions based on both channels together. Keeps your stereo image tighter.
- **Mid-Side** - Process the center and sides of your stereo image separately.

**Utility**
- **Auto Gain** - Automatically compensates for volume loss from clipping. Handy for A/B comparisons.

## Metering

The plugin shows you input and output levels with color-coded meters (green/yellow/red) plus a transfer curve display so you can see exactly what each clipping algorithm is doing to your signal.

## Building

You'll need JUCE (included as a submodule) and Xcode on macOS.

```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/AudioClipper.git

# Open in Projucer and export to Xcode, or just open the Xcode project directly
open Builds/MacOSX/AudioClipper.xcodeproj
```

## Project Layout

```
Source/
  PluginProcessor.cpp  - All the DSP stuff
  PluginEditor.cpp     - UI and meters
JUCE/                  - Framework (submodule)
Builds/MacOSX/         - Xcode project
```

---

Built with [JUCE](https://juce.com/)

