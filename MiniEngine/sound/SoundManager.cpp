#include "stdafx.h"
#include "SoundManager.h"

namespace nsMyGame {

	CSoundManager* CSoundManager::m_soundManager = nullptr;

	void CSoundManager::Init(const wchar_t* filePath, int num, bool flg, SoundType sType, float vol) {

		for (int i = 0; i < m_soundData.size(); i++) {
			if (m_soundData[i].soundNum == num) {

				MessageBoxA(nullptr, "���ɓo�^���ꂽ�T�E���h�̓o�^�ԍ��ł��B", "�G���[", MB_OK);
				return;
			}
		}

		SoundData soundData;
		soundData.soundName = filePath;
		soundData.soundVolume = vol;
		soundData.soundNum = num;
		soundData.roopFlg = flg;
		soundData.soundType = sType;

		m_soundData.push_back(soundData);
	}

	void CSoundManager::Play(int num) {
		bool Flg = false;
		for (int i = 0; i < m_soundData.size(); i++) {
			//�o�^�ԍ���������������Đ��B
			if (m_soundData[i].soundNum == num) {
				m_soundData[i].sound = NewGO<CSoundSource>(enPriority_Zeroth);
				m_soundData[i].sound->Init(m_soundData[i].soundName);
				switch (m_soundData[i].soundType) {
				case SoundType::Type_BGM:
					m_soundData[i].sound->SetVolume(m_soundData[i].soundVolume * m_bgmVolume);
					break;
				case SoundType::Type_SE:
					m_soundData[i].sound->SetVolume(m_soundData[i].soundVolume * m_seVolume);
					break;
				case SoundType::Type_Effect:
					m_soundData[i].sound->SetVolume(m_soundData[i].soundVolume * m_effectVolume);
					break;
				}
				m_soundData[i].sound->Play(m_soundData[i].roopFlg);
				Flg = true;
			}
		}

		if (!Flg) {
			MessageBoxA(nullptr, "�܂����݂��Ȃ��T�E���h�ł��B���Init()�֐����Ă�ł��������B", "�G���[", MB_OK);
			return;
		}
	}

	bool CSoundManager::IsPlaying(int num) {
		for (int i = 0; i < m_soundData.size(); i++) {
			//�o�^�ԍ���������������Đ��B
			if (m_soundData[i].soundNum == num) {
				return m_soundData[i].sound->IsPlaying();
			}
		}


		MessageBoxA(nullptr, "�܂����݂��Ȃ��T�E���h�ł��B���Init()�֐����Ă�ł��������B", "�G���[", MB_OK);
		return false;
	}

	void CSoundManager::SetRoopFlg(int num, bool flg) {

		bool Flg = false;
		for (int i = 0; i < m_soundData.size(); i++) {
			//�o�^�ԍ��������������烋�[�v�t���O��ݒ�B
			if (m_soundData[i].soundNum == num) {

				m_soundData[i].roopFlg = flg;
				Flg = true;
			}
		}

		if (!Flg) {
			MessageBoxA(nullptr, "�܂����݂��Ȃ��T�E���h�ł��B���Init()�֐����Ă�ł��������B", "�G���[", MB_OK);
			return;
		}
	}

	void CSoundManager::Release(int num) {

		for (int i = 0; i < m_soundData.size(); i++) {
			//�o�^�ԍ���������������폜�B
			if (m_soundData[i].soundNum == num) {
				DeleteGO(m_soundData[i].sound);
			}
		}
	}

	void CSoundManager::SetSoundSourceVolume(int num, float vol) {

		bool Flg = false;
		for (int i = 0; i < m_soundData.size(); i++) {
			//�o�^�ԍ���������������{�����[����ݒ�B
			if (m_soundData[i].soundNum == num) {

				m_soundData[i].soundVolume = vol;
				Flg = true;
			}
		}

		if (!Flg) {
			MessageBoxA(nullptr, "�܂����݂��Ȃ��T�E���h�ł��B��ɐ�����Init()�֐����Ă�ł��������B", "�G���[", MB_OK);
			return;
		}
	}

	void CSoundManager::Update() {


	}
}