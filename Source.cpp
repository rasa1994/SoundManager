import Sound;
import <deque>;
import <vector>;
import <memory>;
import <string>;
import <iostream>;
import <thread>;
import <chrono>;
int main()
{
	SoundPtr sound1 = Sound::CreateSound("Music/Sound1.wav");
	SoundPtr sound2 = Sound::CreateSound("Music/Sound2.wav");
	SoundPtr sound3 = Sound::CreateSound("Music/Sound3.wav");

	sound1->Play();
	sound2->Play();
	sound3->Play();

	std::this_thread::sleep_for(std::chrono::seconds(5));
}