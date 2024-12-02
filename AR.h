/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

C++ Real-Time Audio Programming with Bela - Lecture 16: MIDI part 2
*/

// AR.h: header file for defining the AR class

#pragma once

#include "Ramp.h"

class AR {
private:
	// AR state machine variables, used internally
	enum State {
		StateOff = 0,
		StateAttack,
		StateDecay,
		StateSustain,
		StateRelease
	};

public:
	// Constructor
	AR();
	
	// Constructor with argument
	AR(float sampleRate);
	
	// Set the sample rate, used for all calculations
	void setSampleRate(float rate);
	
	// Start the envelope, going to the Attack state
	void trigger();
	
	// Reset the envelope to its initial state
	void reset();
	
	// Calculate the next sample of output, changing the envelope
	// state as needed
	float process(); 
	
	// Indicate whether the envelope is active or not (i.e. in
	// anything other than the Off state
	bool isActive();
	
	// Methods for getting and setting parameters
	float getAttackTime() { return attackTime_; }
	float getReleaseTime() { return releaseTime_; }
	
	void setAttackTime(float attackTime);
	void setReleaseTime(float releaseTime);
	
	// Destructor
	~AR();

private:
	// State variables and parameters, not accessible to the outside world
	float attackTime_;
	float releaseTime_;
	
	State state_;			// Current state of the AR (one of the enum values above)
	Ramp ramp_;				// Line segment generator
};