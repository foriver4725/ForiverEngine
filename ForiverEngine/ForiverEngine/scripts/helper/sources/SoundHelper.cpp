#include "../headers/SoundHelper.h"

// チュートリアルからコピーした
// TODO: 異常系の処理は改善した方がいいと思う
//https://learn.microsoft.com/ja-jp/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2

#include <xaudio2.h>

#ifdef _XBOX
//Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#else
//Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);

namespace ForiverEngine
{
	struct SoundHelper::Impl
	{
		bool initializeSucceeded = false;                 // 全体の初期化が成功したかどうか
		bool coInitialized = false;                       // CoInitialize が成功したかどうか
		IXAudio2* xAudio2 = nullptr;                      // ネイティブのハンドラ
		IXAudio2MasteringVoice* masteringVoice = nullptr; // マスタリングボイスのハンドラ

		Impl()
		{
			if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
				return;
			coInitialized = true;

			if (FAILED(::XAudio2Create(&xAudio2)))
				return;

			if (FAILED(xAudio2->CreateMasteringVoice(&masteringVoice)))
				return;

			initializeSucceeded = true;
		}

		~Impl()
		{
			if (masteringVoice != nullptr)
			{
				masteringVoice->DestroyVoice();
				masteringVoice = nullptr;
			}

			if (xAudio2 != nullptr)
			{
				xAudio2->Release(); // NOTE: 戻り値は無視している
				xAudio2 = nullptr;
			}

			if (coInitialized)
			{
				CoUninitialize();
				coInitialized = false;
			}
		}
	};

	class XAudio2Sound : public ISound
	{
	public:

		// 足りなくなったら増やす
		XAudio2Sound(IXAudio2* xAudio2, int sourceVoiceCapacity) : xAudio2(xAudio2)
		{
			sourceVoices.reserve(sourceVoiceCapacity);
		}

		~XAudio2Sound()
		{
			for (auto& sourceVoice : sourceVoices)
				sourceVoice->Stop();
			sourceVoices.clear();

			delete[] buffer.pAudioData;
		}

		virtual bool Play() override
		{
			IXAudio2SourceVoice* sourceVoice = GetOrCreateSourceVoice();
			if (sourceVoice == nullptr)
				return false;

			// ソース音声に送信
			if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer)))
				return false;

			if (FAILED(sourceVoice->Start()))
				return false;

			return true;
		}

		WAVEFORMATEXTENSIBLE format{};
		XAUDIO2_BUFFER buffer{}; // サウンドの再生が停止するまで解放しない事

	private:
		struct SourceVoiceDeleter
		{
			void operator()(IXAudio2SourceVoice* sourceVoice) const noexcept
			{
				if (sourceVoice)
					sourceVoice->DestroyVoice();
			}
		};

		// 再生中のソースボイスのハンドラを保持する
		// 多重再生出来るようにするため、プーリングする
		using SourceVoicePointer = std::unique_ptr<IXAudio2SourceVoice, SourceVoiceDeleter>;
		std::vector<SourceVoicePointer> sourceVoices{};

		IXAudio2* xAudio2 = nullptr; // ネイティブのハンドラ

		// プールから使えるものを取得する
		// 無ければ、新規作成してプールに追加し、それを返す
		IXAudio2SourceVoice* GetOrCreateSourceVoice()
		{
			// 使えるものがあればそれを返す
			for (auto& sourceVoice : sourceVoices)
			{
				XAUDIO2_VOICE_STATE state{};
				sourceVoice->GetState(&state);

				if (state.BuffersQueued == 0)
					return sourceVoice.get();
			}

			// 使えるものが無ければ、新規作成してプールに追加する
			SourceVoicePointer newSourceVoice = CreateSourceVoice();
			if (newSourceVoice == nullptr)
				return nullptr;

			sourceVoices.push_back(std::move(newSourceVoice));
			return sourceVoices.back().get();
		}

		// 新規作成
		SourceVoicePointer CreateSourceVoice()
		{
			if (xAudio2 == nullptr)
				return nullptr;

			IXAudio2SourceVoice* sourceVoice = nullptr;
			if (FAILED(xAudio2->CreateSourceVoice(&sourceVoice, (WAVEFORMATEX*)&format)))
				return nullptr;

			if (sourceVoice == nullptr)
				return nullptr;

			return SourceVoicePointer(sourceVoice);
		}
	};

	SoundHelper::SoundHelper() : impl(std::make_unique<Impl>()) {}
	SoundHelper::~SoundHelper() = default;

	std::unique_ptr<ISound> SoundHelper::LoadSound(std::string_view path)
	{
		if (!impl->initializeSucceeded)
			return nullptr;

		// ファイルを開く
#ifdef _XBOX
		const CHAR* fileNamePointer = path.data();
#else
		const std::wstring fileName = StringUtils::UTF8ToUTF16(path);
		const WCHAR* fileNamePointer = fileName.c_str();
#endif
		const HANDLE fileHandle = CreateFile(
			fileNamePointer,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);

		if (fileHandle == INVALID_HANDLE_VALUE)
			return nullptr;

		if (SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			return nullptr;

		// "RIFF" チャンクを見つけて、ファイルの種類を確認する
		DWORD chunkSize;
		DWORD chunkDataPosition;
		FindChunk(fileHandle, fourccRIFF, chunkSize, chunkDataPosition);
		DWORD filetype;
		ReadChunkData(fileHandle, &filetype, sizeof(DWORD), chunkDataPosition);
		if (filetype != fourccWAVE)
			return nullptr;

		// 出力サウンド
		std::unique_ptr<XAudio2Sound> sound = std::make_unique<XAudio2Sound>(impl->xAudio2, 4);

		// フォーマット : "fmt" チャンクを見つける
		FindChunk(fileHandle, fourccFMT, chunkSize, chunkDataPosition);
		ReadChunkData(fileHandle, &sound->format, chunkSize, chunkDataPosition);

		// サウンドデータ : "data" チャンクを見つける
		FindChunk(fileHandle, fourccDATA, chunkSize, chunkDataPosition);
		BYTE* buffer = new BYTE[chunkSize]; // NOTE: 利用者側で delete すること
		ReadChunkData(fileHandle, buffer, chunkSize, chunkDataPosition);
		sound->buffer.AudioBytes = chunkSize; // バッファサイズ
		sound->buffer.pAudioData = buffer; // バッファ
		sound->buffer.Flags = XAUDIO2_END_OF_STREAM; // バッファの後には何もない

		// ファイルを閉じる
		::CloseHandle(fileHandle); // NOTE: 戻り値は無視している

		return sound;
	}
}

/// <summary>
/// RIFF (リソース交換ファイル形式) ファイルでチャンクを検索する
/// </summary>
HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;
}

/// <summary>
/// チャンクのデータ セクションの先頭にファイル ポインターを調整することで、そのデータを読み取る
/// </summary>
HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

#ifdef _XBOX
//Big-Endian
#undef fourccRIFF
#undef fourccDATA
#undef fourccFMT
#undef fourccWAVE
#undef fourccXWMA
#undef fourccDPDS
#else
//Little-Endian
#undef fourccRIFF
#undef fourccDATA
#undef fourccFMT
#undef fourccWAVE
#undef fourccXWMA
#undef fourccDPDS
#endif
