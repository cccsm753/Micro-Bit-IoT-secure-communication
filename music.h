#ifndef MUSIC_H  // Include guard
#define MUSIC_H

static Pin *pin = NULL;
static uint8_t pitchVolume = 0xff;


enum Note {
    C = 262,
    CSharp = 277,
    D = 294,
    Eb = 311,
    E = 330,
    F = 349,
    FSharp = 370,
    G = 392,
    GSharp = 415,
    A = 440,
    Bb = 466,
    B = 494,
    C3 = 131,
    CSharp3 = 139,
    D3 = 147,
    Eb3 = 156,
    E3 = 165,
    F3 = 175,
    FSharp3 = 185,
    G3 = 196,
    GSharp3 = 208,
    A3 = 220,
    Bb3 = 233,
    B3 = 247,
    C4 = 262,
    CSharp4 = 277,
    D4 = 294,
    Eb4 = 311,
    E4 = 330,
    F4 = 349,
    FSharp4 = 370,
    G4 = 392,
    GSharp4 = 415,
    A4 = 440,
    Bb4 = 466,
    B4 = 494,
    C5 = 523,
    CSharp5 = 555,
    D5 = 587,
    Eb5 = 622,
    E5 = 659,
    F5 = 698,
    FSharp5 = 740,
    G5 = 784,
    GSharp5 = 831,
    A5 = 880,
    Bb5 = 932,
    B5 = 988,
};

// set the analog pitch for play the music
void analogPitch(int frequency, int ms) {
    if (frequency <= 0 || pitchVolume == 0) {
        pin->setAnalogValue(0);
    } else {
        int v = 1 << (pitchVolume >> 5);
        //
        pin->setAnalogValue(v);
        pin->setAnalogPeriodUs(1000000/frequency);
    }
    if (ms > 0) {
        fiber_sleep(ms);
        pin->setAnalogValue(0);
        fiber_sleep(5);
    }
}



void playScale() {

    const int short_beat = 300;  // Short beat 
    const int long_beat = 800;   // Long beat 

    // Jingle Bells
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, long_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, long_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::G, short_beat);
    analogPitch(Note::C, short_beat);
    analogPitch(Note::D, short_beat);
    analogPitch(Note::E, long_beat);
    fiber_sleep(short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::D, short_beat);
    analogPitch(Note::D, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::D, long_beat);
    analogPitch(Note::G, long_beat);
    fiber_sleep(short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, long_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, long_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::G, short_beat);
    analogPitch(Note::C, short_beat);
    analogPitch(Note::D, short_beat);
    analogPitch(Note::E, long_beat);
    fiber_sleep(short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::E, short_beat);
    analogPitch(Note::G, short_beat);
    analogPitch(Note::G, short_beat);
    analogPitch(Note::F, short_beat);
    analogPitch(Note::D, short_beat);
    analogPitch(Note::C, long_beat);

    fiber_sleep(short_beat);
    
}

#endif // MUSIC_H