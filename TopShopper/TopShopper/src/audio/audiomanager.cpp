#include "audiomanager.h"
#include "core/broker.h"

#include <iostream>
#include <string>
#include <vector>


using namespace std;

AudioManager::SoundEffect* testSoundEffect = new AudioManager::SoundEffect();


AudioManager::AudioManager(Broker *broker) 
	: _broker(broker)
{

}

AudioManager::~AudioManager() {
	// quit SDL_mixer
	Mix_Quit();
}

AudioManager::SoundEffect* AudioManager::getSoundEffect(SoundEffectTypes sound) {
	switch (sound) {
	case SoundEffectTypes::TEST_SOUND:
		return testSoundEffect;
	default:
		return nullptr;
	}
}


Mix_Music* AudioManager::loadMusic(string filename) {
	//string fullPath = SDL_GetBasePath();
	//string fullPath = filename;

	//fullPath.append(filename);

	if (mMusic[filename] == nullptr) {
		mMusic[filename] = Mix_LoadMUS(filename.c_str());
		if (mMusic[filename] == NULL)
			cout << "Music Loading Error" << Mix_GetError() << endl;
	}
	return mMusic[filename];
}

Mix_Chunk* AudioManager::loadSFX(string filename) {
	//string fullPath = SDL_GetBasePath();

	//fullPath.append(filename);
	//printf("%s\n", fullPath.c_str());

	if (mSFX[filename] == nullptr) {
		mSFX[filename] = Mix_LoadWAV(filename.c_str());
		if (mSFX[filename] == NULL) {
			cout << "SFX loading error"<< Mix_GetError()  << endl;
		}
	}
	return mSFX[filename];
}

// play myMusic
void AudioManager::playMusic(Music *myMusic) {
	Mix_VolumeMusic(myMusic->volume);
	//Mix_PlayMusic(myMusic->music, myMusic->loop);
	if (Mix_PlayMusic(myMusic->music, myMusic->loop) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
	}
}

// pause music
void AudioManager::pauseMusic() {
	if (Mix_PlayingMusic() != 0)
		Mix_PauseMusic();
}

// resume the paused music
void AudioManager::resumeMusic() {
	if (Mix_PausedMusic() != 0)
		Mix_ResumeMusic();
}

void AudioManager::playSFX(SoundEffect *mySfx) {
	Mix_SetPosition(mySfx->channel, mySfx->angle, mySfx->distance);
	Mix_PlayChannel(mySfx->channel, mySfx->sfx, mySfx->loop);
	//Mix_PlayChannelTimed(mySfx->channel, mySfx->sfx, mySfx->loop, mySfx->time);
	//SDL_Delay(mySfx->time);
}

void AudioManager::init() {

	// init SDL
	SDL_Init(SDL_INIT_AUDIO);
	// init SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
		cout << "mixer initialization error" << endl;
	}

	// allocate sound effect channel  
	Mix_AllocateChannels(16);

	// load sound effect from file
	for (int i = 0; i < filenames.size(); i++) {
		SoundEffect sfx;
		sfx.filename = filenames[i];
		sfx.sfx = loadSFX(sfx.filename);
		sfx.channel = i; // allocate the channel for sound effect
		soundEffects.push_back(sfx);
	}

	testSoundEffect->filename = "../TopShopper/resources/sfx/test2.wav";
	testSoundEffect->sfx = loadSFX(testSoundEffect->filename);
	testSoundEffect->channel = 0;



	

	// load music from file
	bgm.music = loadMusic(bgm.filename);
	cout << "load bgm file" << endl;
	playMusic(&bgm);


	

}

void AudioManager::updateSeconds(double variableDeltaTime) {
	// play all the sound effects insidethe soundeffect vector
	//for (int i = 0; i < soundEffects.size(); i++) {
	//	playSFX(&soundEffects[i]);
	//}
	//resumeMusic();
	//playSFX(testSoundEffect);
	
	

}

