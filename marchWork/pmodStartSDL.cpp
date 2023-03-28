//#include <SDL/SDL.h>
#include <SDL.h>
// #include <SDL/SDL_audio.h>
#include <SDL_audio.h>

#include <queue>
#include <cmath>

#include <fstream>
#include <string>
#include <iostream>

using namespace std; 

string number;

//const int AMPLITUDE = 7000;  //originally 28000
int AMPLITUDE = 7000;
const int FREQUENCY =44100;  // originally 44100

struct BeepObject
{
    double freq;
    int samplesLeft;
};

class Beeper
{
private:
    double v;
    std::queue<BeepObject> beeps;
public:
    Beeper();
    ~Beeper();
    void beep(double freq, int duration);
    void generateSamples(Sint16 *stream, int length);
    void wait();
};

void audio_callback(void*, Uint8*, int);

Beeper::Beeper()
{
    SDL_AudioSpec desiredSpec;

    desiredSpec.freq = FREQUENCY;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audio_callback;
    desiredSpec.userdata = this;

    SDL_AudioSpec obtainedSpec;

    // you might want to look for errors here
    SDL_OpenAudio(&desiredSpec, &obtainedSpec);

    // start play audio
    SDL_PauseAudio(0);
}

Beeper::~Beeper()
{
    SDL_CloseAudio();
}

void Beeper::generateSamples(Sint16 *stream, int length)
{
    int i = 0;
    while (i < length) {

        if (beeps.empty()) {
            while (i < length) {
                stream[i] = 0;
                i++;
            }
            return;
        }
        BeepObject& bo = beeps.front();

        int samplesToDo = std::min(i + bo.samplesLeft, length);
        bo.samplesLeft -= samplesToDo - i;

        while (i < samplesToDo) {
            stream[i] = AMPLITUDE * std::sin(v * 3 * M_PI / FREQUENCY);
            i++;
            v += bo.freq;
        }

        if (bo.samplesLeft == 0) {
            beeps.pop();
        }
    }
}

void Beeper::beep(double freq, int duration)
{
    BeepObject bo;
    bo.freq = freq;
    bo.samplesLeft = duration * FREQUENCY / 1000;

    SDL_LockAudio();
    beeps.push(bo);
    SDL_UnlockAudio();
}

void Beeper::wait()
{
    int size;
    do {
        //SDL_Delay(1);
        SDL_LockAudio();
        size = beeps.size();
        SDL_UnlockAudio();
    } while (size > 0);

}

void audio_callback(void *_beeper, Uint8 *_stream, int _length)
{
    Sint16 *stream = (Sint16*) _stream;
    int length = _length / 2;
    Beeper* beeper = (Beeper*) _beeper;

    beeper->generateSamples(stream, length);
}



int main(int argc, char* argv[])
{
   
	SDL_Init(SDL_INIT_AUDIO);
	Beeper b;
	
	int duration = 50; 			 //originally 1000
    	//double Hz; 				//originally 440
    	
    	int Hz;
    	
    	int flag=0;
    	
    	const int upperCutoff=7000;
   	const int lowerCutoff=4000;
   	
   	ifstream pmodAmp;	//File stream for pmod1 representing amplitude (volume) values
   	
   	ifstream pmodFreq;	//File stream for pmod2 epresenting frequency (pitch) values
   	
   	ofstream logFile;	//used to display results of testing. 
   	
   	string ampStatus;	//used to display if readings are in range or not. Used for testing. 
   	string freqStatus;

	


    	pmodAmp.open("");
    	pmodFreq.open("/home/kchai/Desktop/pmodReadingFiles/pmodFreq.txt");
    	
    	logFile.open("/home/kchai/Desktop/pmodReadingFiles/logFile.txt",ios::trunc);
    		 

    while(true){	//true	
    
    	//stream in data from both pmods
    	
    	pmodAmp.open("/run/user/1000/gvfs/sftp:host=192.168.20.2/sys/bus/iio/devices/iio:device1/in_proximity_raw");
	pmodAmp>>AMPLITUDE;
	pmodAmp.close();

    	pmodFreq.open("/run/user/1000/gvfs/sftp:host=192.168.20.2/sys/bus/iio/devices/iio:device2/in_proximity_raw");
	pmodFreq>>Hz;
	pmodFreq.close();
    	
	
	

	if ( (AMPLITUDE >= lowerCutoff && AMPLITUDE <= upperCutoff) && (Hz >= lowerCutoff && Hz <= upperCutoff) )		//if both pmod readings are in range
	
		{
		
			
			
			ampStatus= "in range";		//global values within this loops scope. 
			freqStatus= "in range";
		
			//b.beep(9000 - Hz, duration);		//pass data to functions	i.e. do something with the data you're streaming in
    			b.wait();	
	
			flag =1;				//signals that you have completed start up condition
		}
		
	
    	else if	( !(AMPLITUDE >= lowerCutoff && AMPLITUDE <= upperCutoff) && (Hz >= lowerCutoff && Hz <= upperCutoff) && flag==1)
	
		{
			//if AMPLITUDE isn't in range but Hz is and you have already performed your startup procedure. 
			
			//limit AMPLITUDE to either upper or lower cutoff. Stream Hz values as usual 
		
		
		
			if ( AMPLITUDE <= lowerCutoff)		// cap at lowerCutoff		
			
				{
					
					
					int temp=AMPLITUDE;

					AMPLITUDE=lowerCutoff;		 //override current amplitude value read in from file with value of lowerCutoff
									// current Hz value read in from file remains unchanged . Value of freqStatus remains unchanged. ******
									
					ampStatus="capped at  lower cutoff: amplitude before: " + to_string(temp) + " amplitude capped: "+ to_string(AMPLITUDE);			
									
					//b.beep(9000 - Hz, duration);
    					b.wait();

				}
			

		
			else if (AMPLITUDE >= upperCutoff)		//cap at upperCutoff
				{	
				
					
					int temp=AMPLITUDE;

					AMPLITUDE=upperCutoff;		//override current amplitude value read in from file with value of upperCutoff
					
					ampStatus="capped at  upper cutoff: amplitude before: "+to_string(temp)+" amplitude capped: "+to_string(AMPLITUDE);
					
					
					//b.beep(9000 - Hz, duration);
    					b.wait();	
					
				}
	
	
	
		}
		
	else if	( (AMPLITUDE >= lowerCutoff && AMPLITUDE <= upperCutoff) && !(Hz >= lowerCutoff && Hz <= upperCutoff) && flag==1)

		{
	
			//if Hz ins't in range but AMPLITUDE is and you have already performed your startup procedure. 
			
			// limit Hz to either upper of lower cutoff. Stream AMPLITUDE values as usual
			
			

			if ( Hz <= lowerCutoff)	// cap at lowerCutoff
				{	

					int  temp=Hz;

					

					Hz=lowerCutoff;

					freqStatus="capped at  lower cutoff: Frequency before: "+to_string(temp)+" Frequency capped: "+to_string(Hz);
		
									
					//b.beep(9000 - Hz, duration);
    					b.wait();
					
				}
			

			else if (Hz >= upperCutoff)	//cap at upperCutoff
				{
					
					

					int  temp=Hz;

					Hz=upperCutoff;		

					freqStatus="capped at  upper cutoff: Frequency before: "+to_string(temp)+" Frequency capped: "+to_string(Hz);
					
					//b.beep(9000 - Hz, duration);
    					b.wait();				
					
				}
	
	
		}
		
	
	else	{		//if neitherof the pmods are in range return to neutral starting condition
	
			
			
			// don't call sound creation function at all. Lack of sound creation function = lack of sound
		

			 ampStatus= " Idle";		//values carried over into next loop logic until they have a reason to change. Global values within this loop. 
                        freqStatus= "Idle";
			 
			 b.wait();

			flag =0; 
		
			//do nothing until you detect starting procedure. 
	
		}




	cout<<"amplitude:  "<< AMPLITUDE <<"   "<< ampStatus<<" 		frequency:  "<< Hz<<"   "<<freqStatus<<endl;
	logFile<<"amplitude:  "<< AMPLITUDE <<"   "<< ampStatus<<" 		frequency:  "<< Hz<<"   "<<freqStatus<<endl;

    	
    }

	
    	logFile.close();

	
    return 0;
}
