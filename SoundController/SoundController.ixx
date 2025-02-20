export module SoundController;
export import <iostream>;
export import <mutex>;
export import <unordered_map>;
export import <AL/al.h>;
export import <AL/alc.h>;
export import <AL/alext.h>;
export import <functional>;
export import <fstream>;
export import <algorithm>;
export import <ranges>;


export
{
	using uint = unsigned int;
	using ulong = unsigned long;
	using uchar = unsigned char;
	using SoundData = std::vector<char>;

	constexpr auto GetErrorMessage(const uint& error)
	{
		switch (error)
		{
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
		default:
			return "UNKNOWN_AL_ERROR";
		}
	}

	bool GetALError(uint& error)
	{
		error = alGetError();
		return error != AL_NO_ERROR;
	}

	struct SoundInfo
	{
		ALuint source;
		ALuint buffer;
		std::string soundPath;
		std::string soundName;
		SoundInfo() :
			source(0),
			buffer(0)
		{
		}
	};

#pragma pack(1)
	struct WAVHeader
	{
		char riff[4];
		uint32_t chunkSize;
		char wave[4];
		char fmt[4];
		uint32_t subchunk1Size;
		uint16_t audioFormat;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
		char data[4];
		uint32_t dataSize;
	};
#pragma pack()

	using HeaderAndData = std::pair<WAVHeader, SoundData>;

	bool LoadWAVFile(const std::string& filename, WAVHeader& header, std::vector<char>& data)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open()) 
		{
			std::cerr << "Failed to open WAV file: " << filename << std::endl;
			return false;
		}

		file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
		if (std::strncmp(header.riff, "RIFF", 4) != 0 || std::strncmp(header.wave, "WAVE", 4) != 0) 
		{
			std::cerr << "Invalid WAV file format: " << filename << std::endl;
			return false;
		}

		data.resize(header.dataSize);
		file.read(data.data(), header.dataSize);
		return true;
	}

	class ISoundController
	{
	public:
		virtual ~ISoundController() = default;

		virtual void DeleteSource(const SoundInfo& soundInfo, bool deleteFromList = true) = 0;

		virtual void StopSound(const SoundInfo& soundInfo) = 0;

		virtual void PlaySound(const SoundInfo& soundInfo, bool bIsLooping, bool bStop, bool bResetPosition) = 0;

		virtual void SetSourceVolume(const SoundInfo& soundInfo, ALfloat alfVolume) = 0;

		virtual void RewindSource(const SoundInfo& soundInfo) = 0;

		virtual bool IsSourcePlaying(const SoundInfo& soundInfo) const = 0;

		virtual bool IsSoundDataLooping(const SoundInfo& soundInfo) const = 0;

		virtual void BreakLoop(const SoundInfo& soundInfo) = 0;

		virtual ulong GetCurrentPosition(const SoundInfo& soundInfo) const = 0;

		virtual HeaderAndData CreateNewSourceAndBuffer(const std::string& filePath, SoundInfo& soundInfo) = 0;

		virtual void CreateSourceForExistingBuffer(SoundInfo& soundInfo) = 0;

		virtual void FullStopBuffer(const SoundInfo& info) = 0;

		virtual void CreateBuffer(ALuint& source, ALuint& buffer)= 0;

		virtual void CreateSource(SoundInfo& info) = 0;

		virtual void PlaySource(ALuint source) = 0;
	};

	class CSoundController : public ISoundController
	{
	public:

		CSoundController(const CSoundController&) = delete;
		CSoundController(const CSoundController&&) = delete;
		CSoundController& operator=(const CSoundController&) = delete;
		CSoundController& operator=(const CSoundController&&) = delete;
		~CSoundController() override;

		void DeleteSource(const SoundInfo& soundInfo, bool deleteFromList = true) override;

		void StopSound(const SoundInfo& soundInfo) override;

		void PlaySound(const SoundInfo& soundInfo, bool bIsLooping, bool bStop, bool bResetPosition) override;

		void SetSourceVolume(const SoundInfo& soundInfo, ALfloat alfVolume) override;

		void RewindSource(const SoundInfo& soundInfo) override;

		bool IsSourcePlaying(const SoundInfo& soundInfo) const override;

		void CreateSource(SoundInfo& info) override;

		bool IsSoundDataLooping(const SoundInfo& soundInfo) const override;

		void BreakLoop(const SoundInfo& soundInfo) override;

		ulong GetCurrentPosition(const SoundInfo& soundInfo) const override;

		void FullStopBuffer(const SoundInfo& info) override;

		void CreateBuffer(ALuint& source, ALuint& buffer) override;

		HeaderAndData CreateNewSourceAndBuffer(const std::string& filePath, SoundInfo& soundInfo) override;

		void CreateSourceForExistingBuffer(SoundInfo& soundInfo) override;

		void PlaySource(ALuint source) override;

		static ISoundController& Get();

	protected:
		CSoundController();

	private:
		bool m_initialized;
		ALCdevice* m_alcDevice;
		ALCcontext* m_alcContext;

		std::unordered_map<ALuint, std::unordered_map<ALuint, SoundInfo>> m_bufferWithSources;
		std::unordered_map<ALuint, bool> m_shouldUnqueue;
		std::unordered_map<ALuint, std::pair<ALuint, ALuint>> m_buffersWithSources;

		void CreateBuffer(ALuint& alBuffer, SoundInfo& soundInfo) const;

		void DeleteBuffer(const SoundInfo& soundInfo, bool deleteFromList = true);

		void CreateSource(ALuint& alSource, SoundInfo& soundInfo);

		bool LogIfOpenALError(const char* message, const SoundInfo& soundInfo) const;

		int UnqueueBuffer(const ALuint& source);
	};

}




CSoundController::CSoundController() : m_initialized(false), m_alcDevice(nullptr), m_alcContext(nullptr)
{
	m_alcDevice = alcOpenDevice(nullptr);
	if (!m_alcDevice)
	{
		std::cout << "Could not open device" << std::endl;
		return;
	}

	m_alcContext = alcCreateContext(m_alcDevice, nullptr);
	if (!m_alcContext || alcMakeContextCurrent(m_alcContext) == ALC_FALSE)
	{
		if (m_alcContext)
			alcDestroyContext(m_alcContext);

		alcCloseDevice(m_alcDevice);
		return;
	}

	m_initialized = true;
}



CSoundController::~CSoundController()
{
	for (const auto& sources : m_bufferWithSources | std::ranges::views::values)
	{
		for (const auto& source : sources | std::ranges::views::values)
		{
			CSoundController::DeleteSource(source, false);
		}
		if (!sources.empty())
		{
			DeleteBuffer(((sources.begin()))->second, false);
		}
	}

	if (!m_alcContext)
		return;

	alcMakeContextCurrent(nullptr);
	alcDestroyContext(m_alcContext);
	alcCloseDevice(m_alcDevice);
	m_initialized = false;
}



ISoundController& CSoundController::Get()
{
	static CSoundController instance{};
	return instance;
}



void CSoundController::DeleteSource(const SoundInfo& soundInfo, bool deleteFromList)
{
	if (!m_initialized)
		return;

	if (alIsSource(soundInfo.source))
	{
		alDeleteSources(1, &soundInfo.source);
		if (LogIfOpenALError("Could not delete source", soundInfo))
		{
			return;
		}
		if (deleteFromList)
		{
			const auto soundInfoCopy = soundInfo;
			m_bufferWithSources[soundInfo.buffer].erase(soundInfo.source);
			if (m_bufferWithSources[soundInfo.buffer].empty())
				DeleteBuffer(soundInfoCopy);
		}
	}
}



void CSoundController::DeleteBuffer(const SoundInfo& soundInfo, bool deleteFromList)
{
	if (!m_initialized)
		return;

	if (alIsBuffer(soundInfo.buffer))
	{
		alDeleteBuffers(1, &soundInfo.buffer);
		if (LogIfOpenALError("Could not delete buffer", soundInfo))
			return;

		if (deleteFromList)
			m_bufferWithSources.erase(soundInfo.buffer);
	}
}



int CSoundController::UnqueueBuffer(const ALuint& source)
{
	int returnResult{};
	ALint unqueueBuffer{};
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &unqueueBuffer);
	if (unqueueBuffer > 0)
	{
		while (unqueueBuffer > 0)
		{
			ALuint bufferUnqueued{};
			alSourceUnqueueBuffers(source, 1, &bufferUnqueued);
			m_shouldUnqueue[bufferUnqueued] = false;
			returnResult = bufferUnqueued;
			unqueueBuffer--;
		}
	}
	return returnResult;
}

void CSoundController::CreateBuffer(ALuint& alBuffer, SoundInfo& soundInfo) const
{
	if (!m_initialized)
		return;

	alGenBuffers(1, &alBuffer);
	if (LogIfOpenALError("Could not create buffer", soundInfo))
	{
		return;
	}
	soundInfo.buffer = alBuffer;
}



void CSoundController::CreateSource(ALuint& alSource, SoundInfo& soundInfo)
{
	if (!m_initialized)
		return;

	alGenSources(1, &alSource);

	if (LogIfOpenALError("Could not create source", soundInfo))
		return;

#ifndef OPEN_AL_VIRTUALIZATION
	alSourcei(alSource, AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);

	if (LogIfOpenALError("Could not create source", soundInfo))
		return;

#endif

	soundInfo.source = alSource;
	m_bufferWithSources[soundInfo.buffer][alSource] = soundInfo;
}



bool CSoundController::LogIfOpenALError(const char* message, const SoundInfo& soundInfo) const
{
	if (!m_initialized)
		return false;

	if (uint error = 0; GetALError(error))
	{
		std::string soundFullPath = soundInfo.soundPath + "\\" + soundInfo.soundName;
		std::cout << GetErrorMessage(error) << " " << soundFullPath << std::endl;
		return true;
	}
	return false;
}



void CSoundController::StopSound(const SoundInfo& soundInfo)
{
	if (!m_initialized)
		return;

	if (alIsBuffer(soundInfo.buffer))
	{
		std::ranges::for_each(m_bufferWithSources[soundInfo.buffer],[&](const auto& sound)
			{
				alSourcePause(sound.second.source);
				[[maybe_unused]] const auto result = LogIfOpenALError("Problem while trying to stop sound with source", sound.second);
			});
	}
}



void CSoundController::PlaySound(const SoundInfo& soundInfo, const bool bIsLooping, const bool bStop, const bool bResetPosition)
{
	if (!m_initialized)
		return;

	if (!alIsSource(soundInfo.source))
		return;

	if (bStop)
		StopSound(soundInfo);

	if (bResetPosition)
	{
		alSourceRewind(soundInfo.source);
		[[maybe_unused]] const auto result = LogIfOpenALError("Could not reset position of source", soundInfo);
	}

	if (bIsLooping)
	{
		alSourcei(soundInfo.source, AL_LOOPING, AL_TRUE);
		[[maybe_unused]] const auto result = LogIfOpenALError("Could not set looping of source", soundInfo);
	}

	if (!IsSourcePlaying(soundInfo))
	{
		alSourcePlay(soundInfo.source);
	}

	[[maybe_unused]] const auto result = LogIfOpenALError("Could not play source", soundInfo);
}



void CSoundController::RewindSource(const SoundInfo& soundInfo)
{
	if (!m_initialized)
		return;

	if (alIsSource(soundInfo.source))
	{
		alSourceRewind(soundInfo.source);
		[[maybe_unused]] const auto result = LogIfOpenALError("Could not reset position of source", soundInfo);
	}
}



bool CSoundController::IsSourcePlaying(const SoundInfo& soundInfo) const
{
	if (!m_initialized)
		return false;

	if (alIsSource(soundInfo.source))
	{
		ALint state{};
		alGetSourcei(soundInfo.source, AL_SOURCE_STATE, &state);
		if (LogIfOpenALError("Could not check state of source", soundInfo))
			return false;

		if ((state == AL_PLAYING))
			return true;
	}
	return false;
}



void CSoundController::CreateSource(SoundInfo& info)
{
	alGenSources(1, &info.source);
}



void CSoundController::SetSourceVolume(const SoundInfo& soundInfo, const ALfloat alfVolume)
{
	if (!m_initialized)
		return;

	if (alIsSource(soundInfo.source))
	{
		alSourcef(soundInfo.source, AL_GAIN, alfVolume);
		[[maybe_unused]] const auto result = LogIfOpenALError("Could not set volume of source", soundInfo);
	}
}



bool CSoundController::IsSoundDataLooping(const SoundInfo& soundInfo) const
{
	if (!m_initialized)
		return false;

	if (alIsSource(soundInfo.source))
	{
		ALint state{};
		alGetSourcei(soundInfo.source, AL_SOURCE_STATE, &state);
		if (LogIfOpenALError("Could not check state of source", soundInfo))
			return false;

		if (state == AL_LOOPING)
			return true;
	}
	return false;
}



void CSoundController::BreakLoop(const SoundInfo& soundInfo)
{
	if (!m_initialized)
		return;

	if (alIsSource(soundInfo.source))
	{
		alSourcef(soundInfo.source, AL_LOOPING, AL_FALSE);
		[[maybe_unused]] const auto result = LogIfOpenALError("Could not check state of source", soundInfo);
		
		alSourcePlay(soundInfo.source);
		[[maybe_unused]] const auto resultSource = LogIfOpenALError("Could not play source", soundInfo);
	}
}



ulong CSoundController::GetCurrentPosition(const SoundInfo& soundInfo) const
{
	if (!m_initialized)
		return 0;

	ALint alPosition{};
	alGetSourcei(soundInfo.source, AL_BYTE_OFFSET, &alPosition);
	if (LogIfOpenALError("Could not check byte offset of source", soundInfo))
		return 0;
	return alPosition;
}

void CSoundController::FullStopBuffer(const SoundInfo& info)
{
	// TODO: implement
}

void CSoundController::CreateBuffer(ALuint& source, ALuint& buffer)
{
	alGenBuffers(1, &buffer);
	m_buffersWithSources[buffer] = std::pair<ALuint, ALuint>(source, buffer);
	m_shouldUnqueue[buffer] = false;

}



HeaderAndData CSoundController::CreateNewSourceAndBuffer(const std::string& filePath, SoundInfo& soundInfo)
{
	if (!m_initialized)
		return {};

	WAVHeader header{};
	std::vector<char> data{};
	if (!LoadWAVFile(filePath, header, data)) 
	{
		std::cerr << "Failed to load WAV file: " << filePath << std::endl;
		return{};
	}

	ALenum format{};
	if (header.numChannels == 1 && header.bitsPerSample == 8)
		format = AL_FORMAT_MONO8;
	else if (header.numChannels == 1 && header.bitsPerSample == 16)
		format = AL_FORMAT_MONO16;
	else if (header.numChannels == 2 && header.bitsPerSample == 8)
		format = AL_FORMAT_STEREO8;
	else if (header.numChannels == 2 && header.bitsPerSample == 16)
		format = AL_FORMAT_STEREO16;
	else if (header.numChannels == 1 && header.bitsPerSample == 32)
		format = AL_FORMAT_MONO_FLOAT32;
	else if (header.numChannels == 2 && header.bitsPerSample == 32)
		format = AL_FORMAT_STEREO_FLOAT32;
	else
	{
		std::cerr << "Unsupported WAV format: " << filePath << std::endl;
		return{};
	}

	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, data.data(), data.size(), header.sampleRate);

	if (LogIfOpenALError("Could not bind buffer with data", soundInfo)) 
	{
		soundInfo.buffer = soundInfo.source = 0;
		return{};
	}

	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);

	if (LogIfOpenALError("Could not bind source with buffer", soundInfo)) 
	{
		soundInfo.buffer = soundInfo.source = 0;
		return{};
	}

	soundInfo.buffer = buffer;
	soundInfo.source = source;
	soundInfo.soundPath = filePath;
	return { header, data };
}



void CSoundController::CreateSourceForExistingBuffer(SoundInfo& soundInfo)
{
	if (!m_initialized)
		return;

	if (alIsBuffer(soundInfo.buffer))
	{
		ALuint newSource{};
		CreateSource(newSource, soundInfo);
		if (alIsSource(newSource))
		{
			alSourcei(newSource, AL_BUFFER, soundInfo.buffer);
			[[maybe_unused]] const auto result = LogIfOpenALError("Could not bind source with buffer", soundInfo);
		}
	}
}

void CSoundController::PlaySource(ALuint source)
{
	if (alIsSource(source))
		alSourcePlay(source);
}