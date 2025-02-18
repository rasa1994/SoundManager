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
	Sound sound1("Music/Sound1.wav");
	Sound sound2("Music/Sound2.wav");
	Sound sound3("Music/Sound3.wav");

	sound1.Play();
	sound2.Play();
	sound3.Play();

	std::this_thread::sleep_for(std::chrono::seconds(5));
}