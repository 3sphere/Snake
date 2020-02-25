#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <map>
#include <string>

enum class SOUND_ID { EAT_FRUIT, GAME_OVER };

class SoundManager
{
public:
	bool Init();
	bool LoadSoundFromFile(SOUND_ID id, const std::string& filepath);
	void PlaySound(SOUND_ID);
	~SoundManager();
private:
	std::map<SOUND_ID, Mix_Chunk*> SoundMap;
};