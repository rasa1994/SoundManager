export module Sound;
import SoundController;
import <unordered_map>;
import <string>;
import <vector>;
import <deque>;
import <bitset>;

export
{
	constexpr unsigned int MAX_BUFFERS_FOR_QUEUE = 256;

	class Sound
	{
	public:
		Sound(const std::string& soundPath);
		void Play(bool isLooping = false, bool stop = false, bool reset = false) const;
		void Stop() const;
		const SoundInfo& GetSoundInfo() const noexcept { return m_info; }
		bool IsPlaying() const noexcept;
		const WAVHeader& GetSoundFile() const noexcept { return m_soundHeader; }
		void PlaySource() const;

	private:
		std::unordered_map<int, bool> m_isPlaying;
		std::string m_soundPath;
		SoundInfo m_info;
		WAVHeader m_soundHeader;
		std::deque<ALuint> m_buffersForQueue;
		unsigned int bufferToPlay;
	};
}




Sound::Sound(const std::string& soundPath) : m_soundPath(soundPath), m_info{}, m_soundHeader{}
{
	CSoundController::Get().CreateNewSourceAndBuffer(soundPath, m_info);
}

void Sound::Play(bool isLooping, bool stop, bool reset) const
{
	CSoundController::Get().PlaySound(m_info, isLooping, stop, reset);
}

void Sound::Stop() const
{
	CSoundController::Get().StopSound(m_info);
}

bool Sound::IsPlaying() const noexcept
{
	return CSoundController::Get().IsSourcePlaying(m_info);
}


void Sound::PlaySource() const
{
	CSoundController::Get().PlaySource(m_info.source);
}