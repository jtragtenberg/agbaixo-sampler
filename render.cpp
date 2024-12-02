#include <Bela.h>
#include <libraries/Midi/Midi.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <vector>
#include <ctime>
#include "Sampler.h"

// Device for handling MIDI messages
Midi gMidi;
const char* gMidiPort0 = "hw:1,0,0";

// Constants for setting up the sample banks
const int kPercussionSamplers = 8;
const int kBassSamplers = 8;
const int kPercussionSampleCount = 3;

const int kMaxSamplers = 16;
Sampler* gPercussionSamplers[kPercussionSamplers];  // Array of Sampler pointers
Sampler* gBassSamplers[kPercussionSamplers];  // Array of Sampler pointers
int gActiveNoteToSamplerMap[128];  // MIDI note to Sampler index map

// Browser-based GUI to adjust parameters
Gui gGui;
GuiController gGuiController;

// MIDI callback function
void midiEvent(MidiChannelMessage message, void *arg);

bool setup(BelaContext *context, void *userData) {
    // File names for the samples
    std::vector<std::string> percussionFilenames[kPercussionSamplers] = {
        {"percussion1_1.wav", "percussion1_2.wav", "percussion1_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
        {"percussion2_1.wav", "percussion2_2.wav", "percussion2_3.wav"},
    };
    std::vector<std::string> bassFilenames[kBassSamplers] = {
        {"bass1.wav"},
        {"bass2.wav"},
        {"bass3.wav"},
        {"bass4.wav"},
        {"bass5.wav"},
        {"bass6.wav"},
        {"bass7.wav"},
        {"bass8.wav"}
    };
    
    // Initialize samplers and assign MIDI note maps
    for (int i = 0; i < kBassSamplers; ++i) {
        gBassSamplers[i] = new Sampler(bassFilenames[i], context->audioSampleRate);
        gActiveNoteToSamplerMap[60 + i] = i;       // Map MIDI note number 60-67 to bass samplers
        gIsPercussionSamplerMap[60 + i] = false;   // Mark these as bass
    }
    for (int i = 0; i < kPercussionSamplers; ++i) {
        gPercussionSamplers[i] = new Sampler(percussionFilenames[i], context->audioSampleRate);
        gActiveNoteToSamplerMap[68 + i] = i;       // Map MIDI note number 68-75 to percussion samplers
        gIsPercussionSamplerMap[68 + i] = true;    // Mark these as percussion
    }
    
    // Initialize the MIDI device
    if (gMidi.readFrom(gMidiPort0) < 0) {
        rt_printf("Unable to read from MIDI port %s\n", gMidiPort0);
        return false;
    }
    gMidi.writeTo(gMidiPort0);
    gMidi.enableParser(true);
    gMidi.setParserCallback(midiEvent, (void *)gMidiPort0);

    // Set up the GUI
    gGui.setup(context->projectName);
    gGuiController.setup(&gGui, "Sampler Controller");

    // Arguments: name, minimum, maximum, increment, default value
    gGuiController.addSlider("Amplitude Attack time", 0.01, 0.001, 3.0, 0);
    gGuiController.addSlider("Amplitude Release time", 3.0, 0.001, 3.0, 0);

    std::srand(std::time(0)); // Seed the random number generator with the current time
    
    return true;
}

// MIDI note on received
void noteOn(int noteNumber, int velocity) {
    if (velocity > 0 && gActiveNoteToSamplerMap[noteNumber] < kMaxSamplers) {
        int samplerIndex = gActiveNoteToSamplerMap[noteNumber];
        gSamplers[samplerIndex]->trigger();
    }
}

// MIDI note off received
void noteOff(int noteNumber) {
    if (gActiveNoteToSamplerMap[noteNumber] < kMaxSamplers) {
        int samplerIndex = gActiveNoteToSamplerMap[noteNumber];
        gSamplers[samplerIndex]->stop();
    }
}

void render(BelaContext *context, void *userData) {
    // // Retrieve values from the sliders
    // float ampAttackTime = gGuiController.getSliderValue(0);
    // float ampReleaseTime = gGuiController.getSliderValue(1);

    // // Set AR parameters for all samplers
    // for (int i = 0; i < kPercussionSamplers; ++i) {
    //     gSamplers[i]->setAttackTime(ampAttackTime);
    //     gSamplers[i]->setReleaseTime(ampReleaseTime);
    // }
    
    for (unsigned int n = 0; n < context->audioFrames; ++n) {
        float out = 0.0;
        for (int i = 0; i < kBassSamplers; ++i) {
            out += gBassSamplers[i]->process();
        }
        for (int i = 0; i < kPercussionSamplers; ++i) {
            out += gPercussionSamplers[i]->process();
        }

        // Write the sample to every audio output channel
        for (unsigned int channel = 0; channel < context->audioOutChannels; ++channel) {
            audioWrite(context, n, channel, out);
        }
    }
}

// This callback function is called every time a new MIDI message is available
void midiEvent(MidiChannelMessage message, void *arg) {
    if (arg != NULL) {
        rt_printf("Message from MIDI port %s ", (const char*) arg);
    }
    
    message.prettyPrint();
    
    if (message.getType() == kmmNoteOn) {
        int noteNumber = message.getDataByte(0);
        int velocity = message.getDataByte(1);
        
        // Velocity of 0 is really a note off
        if (velocity == 0) {
            noteOff(noteNumber);
        } else {
            noteOn(noteNumber, velocity);
        }
    } else if (message.getType() == kmmNoteOff) {
        int noteNumber = message.getDataByte(0);
        noteOff(noteNumber);
    }
}

void cleanup(BelaContext *context, void *userData) {
    for (int i = 0; i < kPercussionSamplers; ++i) {
        delete gPercussionSamplers[i];
    }
    for (int i = 0; i < kBassSamplers; ++i) {
        delete gBassSamplers[i];
    }
}