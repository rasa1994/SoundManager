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
	class Sound;
	using SoundPtr = std::shared_ptr<Sound>;
	class Sound
	{
	public:
		Sound(const std::string& soundPath);
		~Sound();
		void Play(bool isLooping = false, bool stop = false, bool reset = false) const;
		void Stop() const;
		const SoundInfo& GetSoundInfo() const noexcept { return m_info; }
		bool IsPlaying() const noexcept;
		static SoundPtr CreateSound(const std::string& soundPath);
		[[no_discard]]const WAVHeader& GetHeader() const noexcept { return m_soundHeader.first; }
		[[no_discard]]const SoundData& GetSoundData() const noexcept { return m_soundHeader.second; }
		void ReleaseResource();
	private:
		std::unordered_map<int, bool> m_isPlaying;
		std::string m_soundPath;
		SoundInfo m_info;
		HeaderAndData m_soundHeader;
		std::deque<ALuint> m_buffersForQueue;
		unsigned int bufferToPlay;
	};
}



Sound::Sound(const std::string& soundPath) : m_soundPath(soundPath), m_info{}, m_soundHeader{}
{
	m_soundHeader = std::move(CSoundController::Get().CreateNewSourceAndBuffer(soundPath, m_info));
}



Sound::~Sound()
{
	ReleaseResource();
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



SoundPtr Sound::CreateSound(const std::string& soundPath)
{
	return std::make_shared<Sound>(soundPath);
}



void Sound::ReleaseResource()
{
	if (IsPlaying())
	{
		Stop();
	}

	CSoundController::Get().DeleteSource(m_info);
}
