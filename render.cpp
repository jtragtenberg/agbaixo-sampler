/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

C++ Real-Time Audio Programming with Bela - Lecture 2: Playing recorded samples
sample-player: partially finished example that plays a 
               recorded sound file in a loop.
*/

#include <Bela.h>
#include <libraries/AudioFile/AudioFile.h>
#include <libraries/Midi/Midi.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <cmath>
#include <vector>
#include <ctime>

#include "ADSR.h"

// Device for handling MIDI messages
Midi gMidi;

const char* gMidiPort0 = "hw:1,0,0";

// Handling for multiple MIDI notes
const int kMaxActiveNotes = 16;
int gActiveNotes[kMaxActiveNotes];
int gActiveNoteCount = 0;

// std::string gFilenames[3] = {"1.2.2.1.wav","1.2.2.2.wav","1.2.2.3.wav"};	// Name of the sound file (in project folder)
std::string gFilenames[3] = {"test_random-01.wav","test_random-02.wav","test_random-03.wav"};	// Name of the sound file (in project folder)
std::vector<std::vector<float>> gSampleBuffers(3);				// Buffer that holds the sound file
int gReadPointer = -1;							// Position of the last frame we played 
int gSampleSelector = 0;

// ADSR objects
ADSR gAmplitudeADSR;
float gAmplitude = 0.0;

// Browser-based GUI to adjust parameters
Gui gGui;
GuiController gGuiController;

// MIDI callback function
void midiEvent(MidiChannelMessage message, void *arg);

bool setup(BelaContext *context, void *userData)
{
	// Load the mono sample from storage into a buffer	
	for (int i=0; i<3; i++){
		gSampleBuffers[i] = AudioFileUtilities::loadMono(gFilenames[i]);
	}
	
	// Initialise the MIDI device
	if(gMidi.readFrom(gMidiPort0) < 0) {
		rt_printf("Unable to read from MIDI port %s\n", gMidiPort0);
		return false;
	}
	
	gMidi.writeTo(gMidiPort0);
	gMidi.enableParser(true);
	gMidi.setParserCallback(midiEvent, (void *)gMidiPort0);
	
	// Initialise the ADSR objects
	gAmplitudeADSR.setSampleRate(context->audioSampleRate);
	gAmplitudeADSR.setAttackTime(0.01);
	gAmplitudeADSR.setDecayTime(0.25);
	gAmplitudeADSR.setSustainLevel(0.0);
	gAmplitudeADSR.setReleaseTime(3.0);
	
	// Set up the GUI
	gGui.setup(context->projectName);
	gGuiController.setup(&gGui, "ADSR Controller");	
	
	// Arguments: name, minimum, maximum, increment, default value
	gGuiController.addSlider("Amplitude Attack time", 0.01, 0.001, 3.0, 0);
	gGuiController.addSlider("Amplitude Decay time", 0.25, 0.001, 3.0, 0);
	gGuiController.addSlider("Amplitude Sustain level", 0.0, 0, 1, 0);
	gGuiController.addSlider("Amplitude Release time", 3.0, 0.001, 3.0, 0);
	
	// Check if the load succeeded
	for (int i=0; i<3; i++){
		if(gSampleBuffers[i].size() == 0) {
    		rt_printf("Error loading audio file '%s'\n", gFilenames[i].c_str());
    		return false;
		}
	    rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds), played in the sample-rate %f\n", 
    			gFilenames[i].c_str(), gSampleBuffers[i].size(),
    			gSampleBuffers[i].size() / context->audioSampleRate, context->audioSampleRate);
	}
	
	std::srand(std::time(0)); // Seed the random number generator with the current time

    gReadPointer = -1;

	return true;
}

// MIDI note on received
void noteOn(int noteNumber, int velocity) 
{
	// Check if we have any note slots left
	// if(gActiveNoteCount < kMaxActiveNotes) {
	// 	// Keep track of this note, then play it
	// 	gActiveNotes[gActiveNoteCount++] = noteNumber;
		
	// 	// Map velocity to amplitude on a decibel scale
	// 	// float decibels = map(velocity, 1, 127, -40, 0);
	// 	// gAmplitude = powf(10.0, decibels / 20.0);
	
	// 	// Start the ADSR if this was the first note pressed
	// 	// if(gActiveNoteCount == 1) {
	if (noteNumber == 60){
			gSampleSelector = std::rand() % 3;
			gReadPointer = 0;
			gAmplitudeADSR.trigger();
			// gFilterADSR.trigger();
	}
	// 	// }
		
	// 	//trigger a note
		
	// }
}

// MIDI note off received
void noteOff(int noteNumber)
{
	// bool activeNoteChanged = false;
	
	// // Go through all the active notes and remove any with this number
	// for(int i = gActiveNoteCount - 1; i >= 0; i--) {
	// 	if(gActiveNotes[i] == noteNumber) {
	// 		// Found a match: is it the most recent note?
	// 		if(i == gActiveNoteCount - 1) {
	// 			activeNoteChanged = true;
	// 		}
	
	// 		// Move all the later notes back in the list
	// 		for(int j = i; j < gActiveNoteCount - 1; j++) {
	// 			gActiveNotes[j] = gActiveNotes[j + 1];
	// 		}
	// 		gActiveNoteCount--;
	// 	}
	// }
	
	// if(gActiveNoteCount == 0) {
	// 	// No notes left: stop the ADSR
		gAmplitudeADSR.release();	
	// 	// gFilterADSR.release();
	// }
	// else if(activeNoteChanged) {
	// 	// Update the frequency but don't retrigger
	// 	int mostRecentNote = gActiveNotes[gActiveNoteCount - 1];
		
	// 	// gCentreFrequency.rampTo(calculateFrequency(mostRecentNote), gPortamentoTime);
	// }
}



void render(BelaContext *context, void *userData)
{
	float out = 0;
	// Retrieve values from the sliders
	float ampAttackTime = gGuiController.getSliderValue(0);
	float ampDecayTime = gGuiController.getSliderValue(1);
	float ampSustainLevel = gGuiController.getSliderValue(2);
	float ampReleaseTime = gGuiController.getSliderValue(3);
	
	// Set oscillator, ADSR and filter parameters
	gAmplitudeADSR.setAttackTime(ampAttackTime);
	gAmplitudeADSR.setDecayTime(ampDecayTime);
	gAmplitudeADSR.setSustainLevel(ampSustainLevel);
	gAmplitudeADSR.setReleaseTime(ampReleaseTime);
	
    for(unsigned int n = 0; n < context->audioFrames; n++) {

        // gReadPointer++;
        
		// float amplitude = gAmplitude * gAmplitudeADSR.process();
		
		if(gReadPointer != -1) {
			
    		out = 0.5 * gSampleBuffers[gSampleSelector][gReadPointer] * gAmplitudeADSR.process();
    		gReadPointer++;
    	}
    	if (gReadPointer >= (int)gSampleBuffers[gSampleSelector].size()){
    		gAmplitudeADSR.release();
    		gReadPointer = -1;
    	}
    	

        
		// Write the sample to every audio output channel
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
    		audioWrite(context, n, channel, out);
    	}
    }
}

// This callback function is called every time a new MIDI message is available
// This happens on a different thread than the audio processing

void midiEvent(MidiChannelMessage message, void *arg) {
	// Display the port, if available
	if(arg != NULL) {
		rt_printf("Message from midi port %s ", (const char*) arg);
	}
	
	// Display the message
	message.prettyPrint();
		
	// A MIDI "note on" message type might actually hold a real
	// note onset (e.g. key press), or it might hold a note off (key release).
	// The latter is signified by a velocity of 0.
	if(message.getType() == kmmNoteOn) {
		int noteNumber = message.getDataByte(0);
		int velocity = message.getDataByte(1);
		
		// Velocity of 0 is really a note off
		if(velocity == 0) {
			noteOff(noteNumber);
		}
		else {
			noteOn(noteNumber, velocity);
		}
	}
	else if(message.getType() == kmmNoteOff) {
		// We can also encounter the "note off" message type which is the same
		// as "note on" with a velocity of 0.
		int noteNumber = message.getDataByte(0);
		
		noteOff(noteNumber);
	}
	// else if (message.getType() == kmmPitchBend) {
	// 	int pitchBend = message.getDataByte(0) + (message.getDataByte(1) << 7);
		
	// 	pitchWheel(pitchBend);
	// }
	// else if (message.getType() == kmmControlChange) {
	// 	int ccNumber = message.getDataByte(0);
	// 	int ccValue = message.getDataByte(1);
		
	// 	controlChange(ccNumber, ccValue);
	// }
}

void cleanup(BelaContext *context, void *userData)
{

}
