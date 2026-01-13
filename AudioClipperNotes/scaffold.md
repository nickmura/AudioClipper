 Can remove/ignore:
  - Lines 37-61: MIDI handling (acceptsMidi, producesMidi, isMidiEffect) - not needed for audio effects
  - Lines 69-91: Program management - overkill for a simple clipper initially
  - Line 18: ARA extension - advanced DAW integration, unnecessary
  - Lines 173-184: State save/restore - keep this for later when you add parameters

  Should keep:
  - Lines 94-98: prepareToPlay - you'll need this for oversampling initialization
  - Lines 107-129: Bus layout checking - ensures mono/stereo compatibility
  - Lines 132-159: processBlock - your main DSP loop
